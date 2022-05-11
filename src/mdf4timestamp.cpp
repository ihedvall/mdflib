/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <chrono>
#include "mdf4timestamp.h"
namespace {

std::string TimestampToString(uint64_t time) {
  auto ns = time % 1'000'000'000;
  auto ms = ns / 1'000'000;
  auto sec = time / 1'000'000'000; // Convert to time_t seconds since 1970
  time_t t = static_cast<time_t>(sec);
  struct tm bt{};
  gmtime_s(&bt, &t);
  std::ostringstream date_time;
  date_time << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
      << '.' << std::setfill('0') << std::setw(3) << ms;
  return date_time.str();
}

} // Namespace empty

namespace mdf::detail {

void Mdf4Timestamp::GetBlockProperty(BlockPropertyList &dest) const {

  dest.emplace_back("ISO Time", TimestampToString(time_) );
  dest.emplace_back("TZ Offset [min]", std::to_string(tz_offset_));
  dest.emplace_back("DST Offset [min]", std::to_string(dst_offset_));

  std::ostringstream flags;
  flags << (flags_ & TimestampFlag::kLocalTimestamp ? "Local" : "UTC")
    << "," << (flags_ & TimestampFlag::kTimeOffsetValid ? "Offset" : "No Offset");

  dest.emplace_back("Time Flags", flags.str() );
}

size_t Mdf4Timestamp::Read(std::FILE *file) {
  file_position_ = GetFilePosition(file);
  size_t bytes = ReadNumber(file, time_);
  bytes += ReadNumber(file, tz_offset_);
  bytes += ReadNumber(file, dst_offset_);
  bytes += ReadNumber(file, flags_);
  return bytes;
}
}