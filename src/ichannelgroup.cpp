/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelgroup.h"
#include "util/timestamp.h"

namespace mdf {

SampleRecord IChannelGroup::GetSampleRecord() const {
  SampleRecord record;
  record.timestamp = util::time::TimeStampToNs();
  record.record_id = RecordId();
  record.record_buffer = sample_buffer_;
  return std::move(record);
}

void IChannelGroup::ResetSample() const {
  sample_ = 0;
}
void IChannelGroup::IncrementSample() const {
  ++sample_;
}
size_t IChannelGroup::Sample() const {
  return sample_;
}

}
