/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelgroup.h"

namespace mdf {

SampleRecord IChannelGroup::GetSampleRecord() const {
  SampleRecord record;
  record.timestamp = MdfHelper::NowNs();
  record.record_id = RecordId();
  record.record_buffer = sample_buffer_;
  return record;
}

void IChannelGroup::ClearData() { sample_ = 0; }

void IChannelGroup::IncrementSample() const { ++sample_; }

size_t IChannelGroup::Sample() const { return sample_; }

uint16_t IChannelGroup::Flags() const { return 0; }

void IChannelGroup::Flags(uint16_t flags) {}

char16_t IChannelGroup::PathSeparator() { return u'/'; }

void IChannelGroup::PathSeparator(char16_t path_separator) {}

ISourceInformation *IChannelGroup::CreateSourceInformation() { return nullptr; }

ISourceInformation *IChannelGroup::SourceInformation() const {
  return nullptr;
}
IMetaData *IChannelGroup::CreateMetaData() { return nullptr; }
IMetaData *IChannelGroup::MetaData() const { return nullptr; }

IChannel *IChannelGroup::CreateChannel(const std::string_view &name) {
  auto cn_list = Channels();
  auto itr = std::find_if(cn_list.begin(), cn_list.end(),
                              [&] (const auto* channel) {
    return channel != nullptr &&
           strcmp(channel->Name().c_str(), name.data()) == 0;
                              });
  if (itr != cn_list.end()) {
    return *itr;
  }
  auto* new_channel = CreateChannel();
  if (new_channel != nullptr) {
    new_channel->Name(name.data());
  }
  return new_channel;
}

IChannel *IChannelGroup::GetChannel(const std::string_view &name) const {
  auto cn_list = Channels(); // The list contains the composition channels as
                             // well as the ordinary channels.
  auto itr = std::find_if(cn_list.begin(), cn_list.end(),
                  [&] (const auto* channel) {
                    if (!channel) {
                      return false;
                    }
                    const auto pos = channel->Name().find(name);
                    return pos != std::string::npos;
                  });
  return itr != cn_list.end() ? *itr : nullptr;
}

}  // namespace mdf
