/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "copysampleobserver.h"

#include <cmath>

namespace mdf {
CopySampleObserver::CopySampleObserver(const IDataGroup& data_group,
                                       const IChannelGroup& channel_group,
                                       uint64_t base_time,
                                       MdfWriter& writer,
                                       const IDataGroup& dest_data_group,
                                       const IChannelGroup& dest_channel_group)
: SampleRecordObserver(data_group, channel_group, base_time),
  writer_(writer),
  dest_data_group_(dest_data_group),
  dest_channel_group_(dest_channel_group) {

}

void CopySampleObserver::OnSampleRecord() {
  SampleRecord record;
  GetSampleRecord(record);


  record.record_id = dest_channel_group_.RecordId();
  uint64_t sample_time = record.timestamp; // This is absolute time (ns)
  if (!IgnoreSample(sample_time)) {
    writer_.AddSample(dest_data_group_, dest_channel_group_, sample_time,
                                     std::move(record));
  }
}
void CopySampleObserver::SetTimeRange(double min_time, double max_time) {
  min_time_ = static_cast<uint64_t>(
      GetBaseTime() + std::llround(min_time * 1'000'000'000.0));
  max_time_ = static_cast<uint64_t>(
      GetBaseTime() + std::llround(max_time * 1'000'000'000.0));

}

bool CopySampleObserver::IgnoreSample(uint64_t sample_time) const {
  if (min_time_ < max_time_) {
    if (sample_time < min_time_ || sample_time > max_time_) {
      return true;
    }
  }
  return false;
}

} // mdf