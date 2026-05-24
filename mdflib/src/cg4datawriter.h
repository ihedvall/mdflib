/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <vector>
#include "mdf/idatawriter.h"

namespace mdf {
class IChannelGroup;
}

namespace mdf::detail {

class Cg4DataWriter final : public IDataWriter {
 public:
  explicit Cg4DataWriter(const IChannelGroup& group);
  void Reset() override;
  [[nodiscard]] std::vector<uint8_t>& Buffer() override;
  [[nodiscard]] const std::vector<uint8_t>& Buffer() const override;
  [[nodiscard]] size_t RecordSize() const override;
  [[nodiscard]] bool WriteRawRecord(const void* buffer, size_t length) override;
  [[nodiscard]] bool Commit() override;
  [[nodiscard]] std::vector<DataWriterChannelLayout> ChannelLayouts() const override;
 private:
  const IChannelGroup& group_;
  std::vector<uint8_t> buffer_;
  std::vector<DataWriterChannelLayout> layouts_;
};

} // namespace mdf::detail
