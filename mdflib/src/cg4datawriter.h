/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once


#include <vector>
#include "mdf/idatawriter.h"

namespace mdf {
class IChannelGroup;
}

namespace mdf::detail {

class Cg4DataWriter final : public IDataWriter {
 public:
  explicit Cg4DataWriter(IChannelGroup& group);
  void Reset() override;
  [[nodiscard]] std::vector<uint8_t>& SampleBuffer() override;
  [[nodiscard]] const std::vector<uint8_t>& SampleBuffer() const override;
  [[nodiscard]] size_t RecordSize() const override;
  [[nodiscard]] bool WriteRawRecord(const void* buffer, size_t length) override;
  [[nodiscard]] const std::vector<DataWriterChannelLayout>& ChannelLayouts() const override;
private:
  IChannelGroup& group_;
  //std::vector<uint8_t> buffer_;
  std::vector<DataWriterChannelLayout> layouts_;
  DataWriterChannelLayout master_layout_;
};

} // namespace mdf::detail
