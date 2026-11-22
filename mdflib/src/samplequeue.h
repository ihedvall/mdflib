/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <deque>
#include <atomic>
#include <mutex>
#include <memory>

#include "mdf/samplerecord.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdfwriter.h"
#include "mdf/canmessage.h"
#include "mdf/linmessage.h"
#include "mdf/ethmessage.h"

namespace mdf::detail {

class SampleQueue {
 public:
  SampleQueue() = delete;
  SampleQueue(MdfWriter& writer,
              IDataGroup& data_group);
  virtual ~SampleQueue();

  virtual void TrimQueue(); ///< Trims the sample queue.

  void NofDgBlocks(size_t nof_dg_blocks) { nof_dg_blocks_ = nof_dg_blocks;}
  [[nodiscard]] size_t NofDgBlocks() const {return nof_dg_blocks_;}

  /** \brief Saves the queue to file. */
  virtual void SaveQueue(std::unique_lock<std::mutex>& lock);

  /** \brief Flush the sample queue. */
  virtual void CleanQueue(std::unique_lock<std::mutex>& lock);

  void Reset();
  [[nodiscard]] size_t QueueSize() const;
  void RecalculateTimeMaster();
  void AddSample(const IChannelGroup& group, uint64_t time,
    SampleRecord&& sample_record);
  void SaveSample(const IChannelGroup& group, uint64_t time);
  void SaveCanMessage(const IChannelGroup& group,  uint64_t time,
                                         const CanMessage& msg);
  void SaveLinMessage(const IChannelGroup& group,  uint64_t time,
                             const LinMessage& msg);
  void SaveEthMessage(const IChannelGroup& group,  uint64_t time,
                     const EthMessage& msg);
  void SaveMostMessage(const IChannelGroup& group,  uint64_t time,
                      const IMostEvent& msg);
  void SaveFlexRayMessage(const IChannelGroup& group,  uint64_t time,
                       const IFlexRayEvent& msg);
 protected:
  MdfWriter& writer_;
  IDataGroup& data_group_;
  std::map<uint64_t, const IChannel*> master_channels_; ///< List of master channels
  int64_t time_origin_ = 0; ///< Zero of the master time in ns

  void Open();
  [[nodiscard]] bool IsOpen() const;
  void Close();

  [[nodiscard]] bool IsEmpty() const;;

  void AddSample(SampleRecord&& record);

  void PushSample(const SampleRecord& record) ;
  [[nodiscard]] SampleRecord GetSample();
  void PopSample();
  [[nodiscard]] const std::deque<SampleRecord>& Queue() const {
    return queue_;
  }
  void IncrementNofSamples(uint64_t record_id) const;
  virtual void SetLastPosition(std::streambuf& buffer);
  void RecalculateTime(uint64_t record_id, SampleRecord& sample);
  void SetDataPosition(std::streambuf& file);
 private:
  std::deque<SampleRecord> queue_;
  std::atomic<size_t> size_ = 0; ///< Used to trig flushing to disc.
  size_t nof_dg_blocks_ = 0;


};

}  // namespace mdf::detail

