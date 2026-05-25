/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "datawriter.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatawriter.h"


namespace mdf::detail {

class GroupDataWriter final : public IDataWriter {
 public:
  explicit GroupDataWriter(const IChannelGroup& group)
      : group_(group),
        buffer_(group.GetSampleRecord().record_buffer) {
    const auto channels = group_.Channels();
    layouts_.reserve(channels.size());
    for (size_t index = 0; index < channels.size(); ++index) {
      const auto* channel = channels[index];
      // Do not expose the master channel in the layout list. The master
      // channel value is typically calculated during SaveSample() and
      // should not be written directly by bulk writers unless
      // CalculateMasterTime() is disabled for the channel.
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

  void Reset() override {
    std::fill(buffer_.begin(), buffer_.end(), 0);
  }

  [[nodiscard]] std::vector<uint8_t>& Buffer() override {
    return buffer_;
  }

  [[nodiscard]] const std::vector<uint8_t>& Buffer() const override {
    return buffer_;
  }

  [[nodiscard]] size_t RecordSize() const override {
    return buffer_.size();
  }

  [[nodiscard]] bool WriteRawRecord(const void* buffer, size_t length) override {
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

  [[nodiscard]] SampleRecord Commit() override {
    SampleRecord record = group_.GetSampleRecord();
    record.record_buffer = buffer_;
    return record;
  }

  [[nodiscard]] const std::vector<DataWriterChannelLayout>& ChannelLayouts()
      const override {
    return layouts_;
  }

 private:
  const IChannelGroup& group_;
  std::vector<uint8_t> buffer_;
  std::vector<DataWriterChannelLayout> layouts_;
};

std::unique_ptr<IDataWriter> CreateGroupDataWriter(const IChannelGroup& group) {
  return std::make_unique<GroupDataWriter>(group);
}

} // namespace mdf::detail

