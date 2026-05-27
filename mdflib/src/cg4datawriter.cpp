/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "cg4datawriter.h"

#include <algorithm>

#include "mdf/ichannelgroup.h"
#include "mdf/idatawriter.h"

namespace mdf::detail {

Cg4DataWriter::Cg4DataWriter(IChannelGroup& group)
    : group_(group){
  const auto channels = group_.Channels();
  layouts_.reserve(channels.size());
  for (size_t index = 0; index < channels.size(); ++index) {
    const auto* channel =  channels[index];
    if(channel->Type() == ChannelType::Master) {
      if(channel->ByteOffset()!=0){
        // only support master channel at the beginning of the record buffer
        throw std::runtime_error("Master channel should be at the beginning of the record buffer");
      }
    }
    // if (channel == nullptr || channel->Type() == ChannelType::Master) {
    //   continue;
    // }
    DataWriterChannelLayout layout;
    if(channel->Type() == ChannelType::Master) {
      master_layout_.channel_index = index;
      master_layout_.name = channel->Name();
      master_layout_.byte_offset = channel->ByteOffset();
      master_layout_.data_bytes = channel->DataBytes();
      master_layout_.data_type = channel->DataType();
    }
    else{
      layout.channel_index = index;
      layout.name = channel->Name();
      layout.byte_offset = channel->ByteOffset();
      layout.data_bytes = channel->DataBytes();
      layout.data_type = channel->DataType();
      layouts_.emplace_back(std::move(layout));
    }
    
  }
}

void Cg4DataWriter::Reset() {
  auto& buffer = SampleBuffer();
  std::fill(buffer.begin(), buffer.end(), 0);
}

std::vector<uint8_t>& Cg4DataWriter::SampleBuffer() {
  return group_.SampleBuffer();
}

const std::vector<uint8_t>& Cg4DataWriter::SampleBuffer() const {
  return group_.SampleBuffer();
}

size_t Cg4DataWriter::RecordSize() const {
  return group_.SampleBuffer().size() - master_layout_.data_bytes;
}

bool Cg4DataWriter::WriteRawRecord(const void* buffer, size_t length) {
  auto& sample_buffer = SampleBuffer();
  auto *buffer_ptr = sample_buffer.data()+master_layout_.data_bytes;
  auto buffer_size = RecordSize();
  if (length != buffer_size) {
    return false;
  }
  if (length == 0) {
    return false;
  }
  if (buffer == nullptr) {
    return false;
  }
  std::memcpy(buffer_ptr, buffer, length);
  return true;
}



const std::vector<DataWriterChannelLayout>& Cg4DataWriter::ChannelLayouts() const {
  return layouts_;
}

} // namespace mdf::detail
