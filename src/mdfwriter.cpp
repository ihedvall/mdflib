/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdfwriter.h"

#include <mdf/idatagroup.h>
#include <mdf/mdflogstream.h>
#include <string.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>

#include "mdfblock.h"
#include "platform.h"
#include "dg3block.h"

using namespace std::filesystem;
using namespace std::chrono_literals;

namespace {

std::string StrErrNo(int error) {
  std::string err_str(200, '\0');
  Platform::strerror(error, err_str.data(), err_str.size());
  return err_str;
}

}  // namespace

namespace mdf {

MdfWriter::~MdfWriter() { StopWorkThread(); }

void MdfWriter::PreTrigTime(double pre_trig_time) {
  pre_trig_time *= 1'000'000'000;
  pre_trig_time_ = static_cast<uint64_t>(pre_trig_time);
}

double MdfWriter::PreTrigTime() const {
  auto temp = static_cast<double>(pre_trig_time_);
  temp /= 1'000'000'000;
  return temp;
}

IHeader* MdfWriter::Header() const {
  return mdf_file_ ? mdf_file_->Header() : nullptr;
}

IDataGroup* MdfWriter::CreateDataGroup() {
  return !mdf_file_ ? nullptr : mdf_file_->CreateDataGroup();
}

IChannelGroup* MdfWriter::CreateChannelGroup(IDataGroup* parent) {
  return parent == nullptr ? nullptr : parent->CreateChannelGroup();
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

      detail::OpenMdfFile(file, filename_, "rb");
      if (file != nullptr) {
        mdf_file_->ReadEverythingButData(file);
        std::fclose(file);
        write_state_ = WriteState::Finalize;  // Append to the file
        MDF_DEBUG() << "Reading existing file. File: " << filename_;
        init = true;
      } else {
        MDF_ERROR() << "Failed to open the existing MDF file. File: "
                    << filename_;
        write_state_ = WriteState::Create;
      }
    } else {
      // Create a new file
      write_state_ = WriteState::Create;  // Indicate the file shall be opened
                                          // with "wb" option.
      init = true;
    }
  } catch (const std::exception& err) {
    if (file != nullptr) {
      fclose(file);
      write_state_ = WriteState::Finalize;
      MDF_ERROR() << "Failed to read the existing MDF file. Error: "
                  << err.what() << ", File: " << filename_;
    } else {
      write_state_ = WriteState::Create;
      MDF_ERROR() << "Failed to open the existing MDF file. Error: "
                  << err.what() << ", File: " << filename_;
    }
  }
  return init;
}

bool MdfWriter::InitMeasurement() {
  StopWorkThread();  // Just in case
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  // Set up internal sample buffers so the last channel values can be stored
  const bool prep = PrepareForWriting();
  if (!prep) {
    MDF_ERROR() << "Failed to prepare the file for writing. File: " << filename_;
    return false;
  }
  // 1: Save ID, HD, DG, AT, CG and CN blocks to the file.
  std::FILE* file = nullptr;
  detail::OpenMdfFile(file, filename_,
                      write_state_ == WriteState::Create ? "wb" : "r+b");
  if (file == nullptr) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }

  const bool write = mdf_file_->Write(file);
  SetDataPosition(file); // Set up data position to end of file
  fclose(file);
  start_time_ = 0; // Zero indicate not started
  stop_time_ = 0;  // Zero indicate not stopped
  // Start the working thread that handles the samples
  write_state_ = WriteState::Init;  // Waits for new samples
  work_thread_ = std::thread(&MdfWriter::WorkThread, this);
  return write;
}

void MdfWriter::SaveSample(IChannelGroup& group, uint64_t time) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;

  std::lock_guard lock(locker_);
  sample_queue_.emplace_back(sample);
}

void MdfWriter::StartMeasurement(uint64_t start_time) {
  write_state_ = WriteState::StartMeas;
  start_time_ = start_time;
  stop_time_ = 0; // Zero indicate not stopped
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

  // Save outstanding non-written blocks and any block updates as
  // sample counters which changes during DG/DT updates
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  std::FILE* file = nullptr;
  detail::OpenMdfFile(file, filename_, "r+b");
  if (file == nullptr) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }
  const bool write = mdf_file_ && mdf_file_->Write(file);
  const bool signal_data = WriteSignalData(file);
  fclose(file);
  write_state_ = WriteState::Finalize;
  return write && signal_data;
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
  // Last Time - First Time <= Pre-trig time. Note that we must include start
  // sample, so ignore the last sample
  while (sample_queue_.size() > 1) {
    const auto next_time = sample_queue_[1].timestamp;
    const auto last_time = sample_queue_.back().timestamp;
    if (start_time_ > 0) {
      // Measurement started
      if (next_time >= start_time_ - pre_trig_time_) {
        break;
      }
    } else {
      // Measurement not started. The queue shall be at least the pre-trig time
      const auto buffer_time = last_time - next_time;
      if (buffer_time <= pre_trig_time_) {
        break;
      }
    }
    sample_queue_.pop_front();
  }
}

void MdfWriter::WorkThread() {
  do {
    // Wait on stop condition
    std::unique_lock lock(locker_);
    sample_event_.wait_for(lock, 10s, [&] { return stop_thread_.load(); });
    switch (write_state_) {
      case WriteState::Init: {
        TrimQueue();  // Purge the queue using pre-trig time
        break;
      }
      case WriteState::StartMeas: {
        SaveQueue(lock); // Save the contents of the queue to file
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
  // Save uncompressed data in last DG3 block
  auto *header = Header();
  if (header == nullptr) {
    return;
  }

  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto* dg3 = dynamic_cast<detail::Dg3Block*>(last_dg);
  if (dg3 == nullptr) {
    return;
  }

  lock.unlock();
  std::FILE* file = nullptr;
  Platform::fileopen(&file, filename_.c_str(), "r+b");
  if (file == nullptr) {
    lock.lock();
    return;
  }

  SetLastPosition(file);

  lock.lock();

  // Trim the queue so the start time is included in the first sample
  TrimQueue();

  // Save the queue onto the file
  while (!sample_queue_.empty()) {
    // Write a sample last to file
    auto sample = sample_queue_.front();
    sample_queue_.pop_front();
    if (stop_time_ > 0  && sample.timestamp > stop_time_) {
      break;  // Skip this sample
    }
    lock.unlock();

    if (dg3->NofRecordId() > 0) {
      const auto id = static_cast<uint8_t>(sample.record_id);
      fwrite(&id, 1, 1, file);
    }
    fwrite(sample.record_buffer.data(), 1, sample.record_buffer.size(), file);
    if (dg3->NofRecordId() > 1) {
      const auto id = static_cast<uint8_t>(sample.record_id);
      fwrite(&id, 1, 1, file);
    }
    IncrementNofSamples(sample.record_id);
    lock.lock();
  }

  lock.unlock();
  fclose(file);
  lock.lock();
}

void MdfWriter::CleanQueue(std::unique_lock<std::mutex>& lock) {
  SaveQueue(lock);
}

void MdfWriter::IncrementNofSamples(uint64_t record_id) const {
  auto* header = Header();
  if (header == nullptr) {
    return;
  }
  auto* data_group = header->LastDataGroup();
  if (data_group == nullptr) {
    return;
  }
  const auto list = data_group->ChannelGroups();
  std::for_each(list.cbegin(), list.cend(), [&](auto* group) {
    if (group != nullptr && group->RecordId() == record_id) {
      group->IncrementSample(); // Increment internal sample counter
      group->NofSamples(group->Sample()); // Update block counter
    }
  });
}

IChannel* MdfWriter::CreateChannel(IChannelGroup* parent) {
  return parent == nullptr ? nullptr : parent->CreateChannel();
}

void MdfWriter::SetDataPosition(std::FILE*) {
  // Only needed for MDF4 and uncompressed storage
}
bool MdfWriter::WriteSignalData(std::FILE* file) {
  // Only  supported by MDF4
  return true;
}

std::string MdfWriter::Name() const {
  try {
    path filename(filename_);
    return filename.stem().string();
  } catch (std::exception& err) {
  }
  return {};
}

}  // namespace mdf