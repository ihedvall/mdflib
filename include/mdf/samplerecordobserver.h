/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/isampleobserver.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdfwriter.h"
#include "mdf/samplerecord.h"

namespace mdf {

class SampleRecordObserver : public ISampleObserver {
public:
  SampleRecordObserver() = delete;
  SampleRecordObserver(const IDataGroup& data_group,
                       const IChannelGroup& channel_group,
                       uint64_t base_time);
  bool OnSample(uint64_t sample, uint64_t record_id,
            const std::vector<uint8_t>& record) override;
  virtual void OnSampleRecord() = 0;

  uint64_t GetBaseTime() const { return base_time_; }
  uint64_t GetSampleTime() const { return base_time_; }
  uint64_t GetSampleIndex() const;
  void GetSampleRecord(SampleRecord& sample_record);
  const IChannelGroup& GetChannelGroup() const {
    return channel_group_;
  };

 private:

  const IChannelGroup& channel_group_;
  int64_t current_sample_index_ = -1;
  SampleRecord last_sample_ = {};

  uint64_t base_time_ = 0;
  IChannel* time_channel_ = nullptr;
  uint64_t sample_time_ = 0;

  void HandleSample();
};

}  // namespace mdf


