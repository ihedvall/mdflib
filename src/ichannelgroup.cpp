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

uint16_t IChannelGroup::Flags() {
  return 0;
}

void IChannelGroup::Flags(uint16_t flags) {
}

char16_t IChannelGroup::PathSeparator() {
  return u'/';
}

void IChannelGroup::PathSeparator(char16_t path_separator) {
}

ISourceInformation *IChannelGroup::CreateSourceInformation() {
  return nullptr;
}

const ISourceInformation *IChannelGroup::SourceInformation() const {
  return nullptr;
}

}
