/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <memory>
#include <filesystem>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <util/logstream.h>
#include "mdf/mdfwriter.h"
#include "iblock.h"


using namespace std::filesystem;
using namespace util::log;
using namespace std::chrono_literals;

namespace {

std::string StrErrNo(errno_t error) {
  std::string err_str(200,'\0');
  strerror_s(err_str.data(), err_str.size(), error);
  return err_str;
}

}
namespace mdf {

MdfWriter::~MdfWriter() {
  StopWorkThread();
}

void MdfWriter::PreTrigTime(double pre_trig_time) {
  auto temp = static_cast<uint64_t>(pre_trig_time);
  temp += 1'000'000'000;
  pre_trig_time_ = temp;
}

IHeader *MdfWriter::Header() const {
  return mdf_file_  ? mdf_file_->Header() : nullptr;
}

IDataGroup *MdfWriter::CreateDataGroup() {
  return !mdf_file_ ? nullptr : mdf_file_->CreateDataGroup();
}

bool MdfWriter::Init(const std::string& filename) {
  bool init = false;
  CreateMdfFile();
  filename_ = filename;
  if (mdf_file_) {
    mdf_file_->FileName(filename);
  }
  std::FILE* file = nullptr;
  try {
    if (std::filesystem::exists(filename_)) {
      // Read in existing file so we can append to it

      detail::OpenMdfFile(file,filename_, "rb");
      if (file != nullptr) {
        mdf_file_->ReadEverythingButData(file);
        std::fclose(file);
        write_state_ = WriteState::Finalize; // Append to the file
        LOG_DEBUG() << "Reading existing file. File: " << filename_;
        init = true;
      } else {
        LOG_ERROR() << "Failed to open the existing MDF file. File: " << filename_;
        write_state_ = WriteState::Create;
      }
    } else {
      // Create a new file
      write_state_ = WriteState::Create; // Indicate the file shall be opened with "wb" option.
      init = true;
    }
  } catch (const std::exception& err) {
    if (file != nullptr) {
      fclose(file);
      write_state_ = WriteState::Finalize;
      LOG_ERROR() << "Failed to read the existing MDF file. Error: " << err.what()
                  << ", File: " << filename_;
    } else {
      write_state_ = WriteState::Create;
      LOG_ERROR() << "Failed to open the existing MDF file. Error: " << err.what()
                  << ", File: " << filename_;
    }
  }
  return init;
}

bool MdfWriter::InitMeasurement() {
  StopWorkThread(); // Just in case
  if (!mdf_file_) {
    LOG_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  // 1: Save ID, HD, DG, AT, CG and CN blocks to the file.
  std::FILE* file = nullptr;
  detail::OpenMdfFile(file, filename_, write_state_ == WriteState::Create ? "wb" : "r+b");
  if (file == nullptr) {
    LOG_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }

  const bool write =  mdf_file_->Write(file);
  fclose(file);

  // Start the working thread that handles the samples
  write_state_ = WriteState::Init; // Waits for new samples
  work_thread_ = std::thread(&MdfWriter::WorkThread, this);
  return write;
}

void MdfWriter::SaveSample(IChannelGroup& group, uint64_t time ) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;

  std::lock_guard lock(locker_);
  sample_queue_.push_back(sample);
}

void MdfWriter::StartMeasurement(uint64_t start_time) {
  write_state_ = WriteState::StartMeas;
  start_time_ = start_time;
  sample_event_.notify_one();

  // Set the time in the header if this is the first DG block in the file.
  // This gives a better start time than when the file was created.
  auto* header = Header();
  if (header == nullptr) {
    return;
  }
  auto dg_list = header->DataGroups();
  if (dg_list.size() == 1) {
    header->StartTime(start_time);
  }

  sample_event_.notify_one();
}

void MdfWriter::StopMeasurement(uint64_t stop_time) {
  write_state_ = WriteState::StopMeas;
  stop_time_ = stop_time;
  sample_event_.notify_one();
}

bool MdfWriter::FinalizeMeasurement() {
  StopWorkThread();

  // Save outstanding SR and any legal updates
  if (!mdf_file_) {
    LOG_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  std::FILE* file = nullptr;
  detail::OpenMdfFile(file, filename_, "r+b");
  if (file == nullptr) {
    LOG_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }
  const bool write = mdf_file_ && mdf_file_->Write(file);
  fclose(file);
  write_state_ = WriteState::Finalize;
  return write;
}

void MdfWriter::StopWorkThread() {
  stop_thread_ = true;
  if (work_thread_.joinable()) {
    sample_event_.notify_one();
    work_thread_.join();
  }
  stop_thread_ = false;
}

void MdfWriter::TrimQueue() {
  // Last Time - First Time <= Pre-trig time
  while (sample_queue_.size() > 2) {
    const auto& last_sample = sample_queue_.back();
    const auto& first_sample = sample_queue_.front();
    auto buffer_time = last_sample.timestamp - first_sample.timestamp;
    if (buffer_time > pre_trig_time_) {
      sample_queue_.pop_front();
    }
  }
}

void MdfWriter::WorkThread() {
  do {
    // Wait on stop condition
    std::unique_lock lock(locker_);
    sample_event_.wait_for(lock,10s, [&] {
      return stop_thread_.load();
    });
    switch (write_state_) {
      case WriteState::Init: {
        TrimQueue(); // Purge the queue using pre-trig time
        break;
      }
      case WriteState::StartMeas: {
        SaveQueue(lock);
        break;
      }

      case WriteState::StopMeas: {
        CleanQueue(lock);
        break;
      }

      default:
        sample_queue_.clear();
        break;
    }
  } while (!stop_thread_);
  {
    std::unique_lock lock(locker_);
    CleanQueue(lock);
  }
}

void MdfWriter::SaveQueue(std::unique_lock<std::mutex>& lock) {
  lock.unlock();
  std::FILE* file = nullptr;
  fopen_s(&file, filename_.c_str(), "r+b");
  if (file == nullptr) {
    lock.lock();
    return;
  }

  SetLastPosition(file);

  lock.lock();

  uint64_t start_time = start_time_;
  start_time -= pre_trig_time_;

  while (sample_queue_.size() > 1) {
    // Write a sample last to file
    auto sample = sample_queue_.front();
    sample_queue_.pop_front();
    const auto& next = sample_queue_.front();
    if (next.timestamp < start_time) {
      continue; // Skip this sample
    }

    lock.unlock();

    if (sample.record_id > 0 ) {
      const auto id = static_cast<uint8_t>(sample.record_id);
      fwrite(&id,1,1,file);
    }
    fwrite(sample.record_buffer.data(),1,sample.record_buffer.size(),file);
    IncrementNofSamples(sample.record_id);
    lock.lock();
  }

  lock.unlock();
  fclose(file);
  lock.lock();

}

void MdfWriter::CleanQueue(std::unique_lock<std::mutex>& lock) {
  uint64_t start_time = start_time_;
  start_time -= pre_trig_time_;

  while (!sample_queue_.empty()) {
    const auto& sample = sample_queue_.front();

    if (sample.timestamp >= start_time && sample.timestamp <= stop_time_) {
      break;
    }
    sample_queue_.pop_front();
  }

  if (sample_queue_.empty()) {
    return;
  }

  lock.unlock();
  std::FILE* file = nullptr;
  fopen_s(&file, filename_.c_str(), "r+b");
  if (file == nullptr) {
    lock.lock();
    return;
  }
  SetLastPosition(file);
  lock.lock();

  while (!sample_queue_.empty()) {
    // Write a sample last to file
    auto sample = sample_queue_.front();
    sample_queue_.pop_front();
    if (sample.timestamp > stop_time_) {
      continue; // Skip this sample
    }

    lock.unlock();
    if (sample.record_id > 0 ) {
      const auto id = static_cast<uint8_t>(sample.record_id);
      fwrite(&id,1,1,file);
    }
    fwrite(sample.record_buffer.data(),1,sample.record_buffer.size(),file);
    IncrementNofSamples(sample.record_id);
    lock.lock();
  }
  lock.unlock();
  fclose(file);
  lock.lock();
}

void MdfWriter::IncrementNofSamples(uint64_t record_id) const {
  auto *header = Header();
  if (header == nullptr) {
    return;
  }
  auto *data_group = header->LastDataGroup();
  if (data_group == nullptr) {
    return;
  }

  std::ranges::for_each(data_group->ChannelGroups(), [&](auto *group) {
    if (group != nullptr && group->RecordId() == record_id) {
      group->IncrementSample();
      group->NofSamples(group->Sample());
    }
  });
}
} // end namespace