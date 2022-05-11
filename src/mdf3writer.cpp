/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <cstdio>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <ranges>
#include "util/logstream.h"
#include "mdf/mdf3writer.h"

#include "cc3block.h"
#include "cn3block.h"
#include "cg3block.h"
#include "dg3block.h"
#include "mdf3file.h"

using namespace util::log;
using namespace std::chrono_literals;

namespace mdf {

Mdf3Writer::Mdf3Writer(const std::string &filename)
: mdf_file_(std::make_unique<detail::Mdf3File>()),
  filename_(filename) {
  std::FILE* file = nullptr;
  try {
    if (std::filesystem::exists(filename)) {
      // Read in existing file so we can append to it
      LOG_INFO() << "Reading existing file. File: " << filename;
      file = std::fopen(filename.c_str(), "rb");
      if (file != nullptr) {
        mdf_file_->ReadEverythingButData(file);
        std::fclose(file);
        write_state_ = WriteState::Finalize; // Append to the file
      } else {
        write_state_ = WriteState::Create;
      }
    } else {
      // Create a new file
      write_state_ = WriteState::Create; // Indicate the file shall be opened with "wb" option.
    }
  } catch (const std::exception& ) {
    if (file != nullptr) {
      fclose(file);
      write_state_ = WriteState::Finalize;
    } else {
      write_state_ = WriteState::Create;
    }
  }
}

Mdf3Writer::~Mdf3Writer() {
  StopWorkThread();
}

IHeader *Mdf3Writer::Header() const {
  auto* mdf3 = dynamic_cast<detail::Mdf3File*> (mdf_file_.get());
  if (mdf3 == nullptr) {
    return nullptr;
  }
  return mdf3->Header();
}

IDataGroup *Mdf3Writer::CreateDataGroup() {
  auto* mdf3 = dynamic_cast<detail::Mdf3File*> (mdf_file_.get());
  if (mdf3 == nullptr) {
    return nullptr;
  }
  return mdf3->CreateDataGroup();
}

IChannelGroup* Mdf3Writer::CreateChannelGroup(IDataGroup* parent) {
  auto* dg3 = dynamic_cast<detail::Dg3Block*> (parent);
  if (dg3 != nullptr) {
    auto cg3 = std::make_unique<detail::Cg3Block>();
    cg3->Init(*dg3);
    dg3->AddCg3(cg3);
  }
  return dg3 != nullptr ? dg3->Cg3().back().get() : nullptr;
}

IChannel *Mdf3Writer::CreateChannel(IChannelGroup *parent) {
  auto* cg3 = dynamic_cast<detail::Cg3Block*> (parent);
  if (cg3 != nullptr) {
    auto cn3 = std::make_unique<detail::Cn3Block>();
    cn3->Init(*cg3);
    cg3->AddCn3(cn3);
  }
  return cg3 != nullptr ? cg3->Cn3().back().get() : nullptr;
}

IChannelConversion *Mdf3Writer::CreateChannelConversion(IChannel *parent) {
  auto *cn3 = dynamic_cast<detail::Cn3Block *> (parent);
  if (cn3 != nullptr) {
    auto cc3 = std::make_unique<detail::Cc3Block>();
    cc3->Init(*cn3);
    cn3->AddCc3(cc3);
  }
  return cn3 != nullptr ? cn3->Cc3() : nullptr;
}

bool Mdf3Writer::InitMeasurement() {
  StopWorkThread(); // Just in case
  if (!mdf_file_) {
    LOG_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }
  auto* mdf3 = dynamic_cast<detail::Mdf3File*> (mdf_file_.get());
  if (mdf3 == nullptr) {
    LOG_ERROR() << "This is not an MDF3 file. Invalid use of the function.";
    return false;
  }

  // 1: Save ID, HD, DG, CG and CN blocks to the file.
  auto* file = std::fopen(filename_.c_str(),
                          write_state_ == WriteState::Create ? "wb" : "r+b"); // Note: Existing file will be lost
  if (file == nullptr) {
    LOG_ERROR() << "Failed to open the file for writing. Error: " << std::strerror(errno)
      << ", File: " << filename_;
    return false;
  }

  const bool write =  mdf3->Write(file);
  fclose(file);

  // Start the working thread that handles the samples
  write_state_ = WriteState::Init; // Waits for new samples
  work_thread_ = std::thread(&Mdf3Writer::WorkThread, this);
  return write;
}

bool Mdf3Writer::FinalizeMeasurement() {
  StopWorkThread();

  // Save outstanding SR and any legal updates
  if (!mdf_file_) {
    LOG_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }
  // 1: Save ID, HD, DG, CG and CN blocks to the file.
  auto* file = std::fopen(filename_.c_str(), "r+b");
  if (file == nullptr) {
    LOG_ERROR() << "Failed to open the file for writing. Error: " << std::strerror(errno)
                << ", File: " << filename_;
    return false;
  }
  auto* mdf3 = dynamic_cast<detail::Mdf3File*> (mdf_file_.get());
  const bool write = mdf3 != nullptr? mdf3->Write(file) : false;
  fclose(file);
  write_state_ = WriteState::Finalize;
  return write;
}

void Mdf3Writer::WorkThread() {
  do {
    // Wait on stop condition
    std::unique_lock lock(locker_);
    sample_event_.wait_for(lock,10s, [&] {return stop_thread_.load();});
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

void Mdf3Writer::StopWorkThread() {
  stop_thread_ = true;
  if (work_thread_.joinable()) {
    sample_event_.notify_one();
    work_thread_.join();
  }
  stop_thread_ = false;
}

void Mdf3Writer::StartMeasurement(uint64_t start_time) {
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

void Mdf3Writer::StopMeasurement(uint64_t stop_time) {
  write_state_ = WriteState::StopMeas;
  stop_time_ = stop_time;

  sample_event_.notify_one();
}

void Mdf3Writer::SaveSample(IChannelGroup& group, uint64_t time ) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;

  std::lock_guard lock(locker_);
  sample_queue_.push_back(sample);
}


void Mdf3Writer::TrimQueue() {
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

void Mdf3Writer::SaveQueue(std::unique_lock<std::mutex>& lock) {
  lock.unlock();
  std::FILE* file = fopen(filename_.c_str(), "r+b");
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

void Mdf3Writer::CleanQueue(std::unique_lock<std::mutex>& lock) {
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
  std::FILE* file = fopen(filename_.c_str(), "r+b");
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

void Mdf3Writer::IncrementNofSamples(uint64_t record_id) const {
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

void Mdf3Writer::SetLastPosition(std::FILE *file) {
#if (MSVC)
  _fseeki64(&file, 0, SEEK_END);
#else
  fseeko64(file, 0, SEEK_END);
#endif

  auto* header = Header();
  if (header == nullptr) {
    return;
  }
  auto* last_dg = header->LastDataGroup();
  if (last_dg == nullptr)  {
    return;
  }
  auto* dg3 = dynamic_cast<detail::Dg3Block*>(last_dg);
  if (dg3 == nullptr) {
    return;
  }

  if (dg3->Link(3) > 0) {
    return;
  }

  dg3->SetLastFilePosition(file);
  auto position = detail::GetFilePosition(file);
  dg3->UpdateLink(file,3, position);
  dg3->SetLastFilePosition(file);
}

} // end namespace mdf