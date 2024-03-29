/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf4timestamp.h"

#include <chrono>
namespace {

std::string TimestampToString(uint64_t time) {
  auto ns = time % 1'000'000'000;
  auto ms = ns / 1'000'000;
  auto sec = time / 1'000'000'000;  // Convert to time_t seconds since 1970
  time_t t = static_cast<time_t>(sec);
  const struct tm *bt = gmtime(&t);
  std::ostringstream date_time;
  date_time << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(3) << ms;
  return date_time.str();
}

}  // namespace

namespace mdf::detail {

void Mdf4Timestamp::GetBlockProperty(BlockPropertyList &dest) const {
  dest.emplace_back("ISO Time", TimestampToString(time_));
  dest.emplace_back("TZ Offset [min]", std::to_string(tz_offset_));
  dest.emplace_back("DST Offset [min]", std::to_string(dst_offset_));

  std::ostringstream flags;
  flags << (flags_ & TimestampFlag::kLocalTimestamp ? "Local" : "UTC") << ","
        << (flags_ & TimestampFlag::kTimeOffsetValid ? "Offset" : "No Offset");

  dest.emplace_back("Time Flags", flags.str());
}

size_t Mdf4Timestamp::Read(std::FILE *file) {
  file_position_ = GetFilePosition(file);
  size_t bytes = ReadNumber(file, time_);
  bytes += ReadNumber(file, tz_offset_);
  bytes += ReadNumber(file, dst_offset_);
  bytes += ReadNumber(file, flags_);
  return bytes;
}

size_t Mdf4Timestamp::Write(std::FILE *file) {
  if (file_position_ <= 0) {
    file_position_ = GetFilePosition(file);
  } else {
    SetFilePosition(file, file_position_);
  }
  auto bytes = WriteNumber(file, time_);
  bytes += WriteNumber(file, tz_offset_);
  bytes += WriteNumber(file, dst_offset_);
  bytes += WriteNumber(file, flags_);
  return bytes;
}

uint64_t Mdf4Timestamp::NsSince1970() const {
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
void Mdf4Timestamp::NsSince1970(uint64_t utc) {
  time_ = utc;
  tz_offset_ = 0;
  dst_offset_ = 0;
  flags_ = 0;
}
}  // namespace mdf::detail