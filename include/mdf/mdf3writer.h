/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include <deque>
#include <vector>
#include <condition_variable>
#include "util/timestamp.h"
#include "mdf/mdffile.h"
#include "mdf/iheader.h"
#include "mdf/idatagroup.h"
#include "mdf/samplerecord.h"

namespace mdf {

class Mdf3Writer {
 public:
  explicit Mdf3Writer(const std::string &filename); ///< Constructor that and creates the ID and HD block.
  virtual~Mdf3Writer(); ///< Destructor that close any open file and destructs.
  Mdf3Writer() = delete;

  [[nodiscard]] bool IsFileNew() const {
    return write_state_ == WriteState::Create;
  }

  void PreTrigTime(double pre_trig_time) {
    uint64_t temp = static_cast<uint64_t>(pre_trig_time);
    temp += 1'000'000'000;
    pre_trig_time_ = temp;
  }

  IHeader* Header() const;
  IDataGroup* CreateDataGroup();
  IChannelGroup* CreateChannelGroup(IDataGroup* parent);
  IChannel* CreateChannel(IChannelGroup* parent);
  IChannelConversion* CreateChannelConversion(IChannel* parent);

  bool InitMeasurement();
  void StartMeasurement(uint64_t time = util::time::TimeStampToNs());
  void StopMeasurement(uint64_t time = util::time::TimeStampToNs());
  void SaveSample(IChannelGroup& group, uint64_t time = util::time::TimeStampToNs() );
  bool FinalizeMeasurement();

private:
  std::unique_ptr<MdfFile> mdf_file_;
  std::string filename_;
  std::thread work_thread_;
  std::atomic_bool stop_thread_ = false;
  std::mutex locker_;
  std::condition_variable sample_event_;

  enum class WriteState : uint8_t {
    Create,       // Only at first measurement
    Init,         // Start work thread and start collecting samples
    StartMeas,    // Start saving samples to file
    StopMeas,     // Stop saving samples. OK to
    Finalize      // OK to add new DG and CG blocks
  };

  std::atomic<WriteState> write_state_ = WriteState::Create;
  using SampleQueue = std::deque<SampleRecord>;
  std::atomic<uint16_t> pre_trig_time_ = 0;   // ns diff
  std::atomic<uint64_t> start_time_ = 0; // ns since 1970
  std::atomic<uint64_t> stop_time_ = 0;
  SampleQueue sample_queue_;

  void WorkThread();
  void StopWorkThread();
  void TrimQueue();
  void SaveQueue(std::unique_lock<std::mutex>& lock);
  void CleanQueue(std::unique_lock<std::mutex>& lock);
  void IncrementNofSamples(uint64_t record_id) const;
  void SetLastPosition(std::FILE* file);
};

} // end namespace mdf

