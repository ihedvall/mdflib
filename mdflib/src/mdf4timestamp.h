/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>

#include "imdftimestamp.h"
#include "mdf/itimestamp.h"
#include "mdf/mdfhelper.h"
#include "mdfblock.h"

namespace mdf::detail {

namespace TimestampFlag {
constexpr uint8_t kUtcTimestamp = 0x00;
constexpr uint8_t kLocalTimestamp = 0x01;
constexpr uint8_t kTimeOffsetValid = 0x02;
}  // namespace TimestampFlag

class Mdf4Timestamp : public IMdfTimeStamp {
 public:
  Mdf4Timestamp();
  void GetBlockProperty(BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  void SetTime(uint64_t time) override;
  void SetTime(ITimestamp &timestamp) override;
  
  [[nodiscard]] std::string GetTimeString() const;
  [[nodiscard]] uint64_t GetTime() const override;

 private:
  uint64_t time_;
  int16_t tz_offset_;
  int16_t dst_offset_;
  uint8_t flags_;
};

}  // Namespace mdf::detail
