/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "copysampleobserver.h"

#include <iostream>
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
   uint64_t sample_time = record.timestamp;
   writer_.AddSample(dest_data_group_, dest_channel_group_, sample_time,
                                   std::move(record));
}

} // mdf