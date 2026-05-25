/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "cg4datawriter.h"

#include <algorithm>

#include "mdf/ichannelgroup.h"
#include "mdf/idatawriter.h"

namespace mdf::detail {

Cg4DataWriter::Cg4DataWriter(const IChannelGroup& group)
    : group_(group), buffer_(group.GetSampleRecord().record_buffer) {
  const auto channels = group_.Channels();
  layouts_.reserve(channels.size());
  for (size_t index = 0; index < channels.size(); ++index) {
    const auto* channel =  channels[index];
    if (channel == nullptr || channel->Type() == ChannelType::Master) {
      continue;
    }
    DataWriterChannelLayout layout;
    layout.channel_index = index;
    layout.name = channel->Name();
    layout.byte_offset = channel->ByteOffset();
    layout.data_bytes = channel->DataBytes();
    layout.data_type = channel->DataType();
    layouts_.emplace_back(std::move(layout));
  }
}

void Cg4DataWriter::Reset() {
  std::fill(buffer_.begin(), buffer_.end(), 0);
}

std::vector<uint8_t>& Cg4DataWriter::Buffer() {
  return buffer_;
}

const std::vector<uint8_t>& Cg4DataWriter::Buffer() const {
  return buffer_;
}

size_t Cg4DataWriter::RecordSize() const {
  return buffer_.size();
}

bool Cg4DataWriter::WriteRawRecord(const void* buffer, size_t length) {
  if (length != buffer_.size()) {
    return false;
  }
  if (length == 0) {
    return true;
  }
  if (buffer == nullptr) {
    return false;
  }
  std::memcpy(buffer_.data(), buffer, length);
  return true;
}

SampleRecord Cg4DataWriter::Commit() {
  SampleRecord record;
  record.timestamp = MdfHelper::NowNs();
  record.record_id = group_.RecordId();
  record.record_buffer = buffer_;
  return record;
}

const std::vector<DataWriterChannelLayout>& Cg4DataWriter::ChannelLayouts() const {
  return layouts_;
}

} // namespace mdf::detail
