/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <util/timestamp.h>
#include "iblock.h"


namespace mdf::detail {

namespace TimestampFlag {
constexpr uint8_t kLocalTimestamp = 0x01;
constexpr uint8_t kTimeOffsetValid = 0x02;
}

class Mdf4Timestamp : public IBlock {
 public:
  void GetBlockProperty(BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;
  [[nodiscard]] uint64_t NsSince1970() const {
    if (flags_ & TimestampFlag::kLocalTimestamp) {
      // Do not know how to convert but use PC local time
      const uint64_t ns = time_ % 1'000'000'000;
      const auto local_time = static_cast<time_t>(time_ / 1'000'000'000);
      auto temp = *std::gmtime(&local_time);
      temp.tm_isdst = -1;
      const auto utc = std::mktime(&temp);
      uint64_t time = utc;
      time *= 1'000'000'000;
      time += ns;
      return time;
    }
    return time_;
  }

  void NsSince1970(uint64_t utc) {
    time_ = utc;
    tz_offset_ = 0;
    dst_offset_ = 0;
    flags_ = 0;
  }

 private:
  uint64_t time_ = 0;      ///< Time in nanoseconds since 1970 also known as UNIX time
  int16_t tz_offset_ = 0;  ///< Time zone offsets in minutes. Best practice is to always use UTC.
  int16_t dst_offset_ = 0; ///< DST offset in minutes. Best practice is to always use UTC.
  uint8_t flags_ = 0;
};

} // Namespace mdf::detail



