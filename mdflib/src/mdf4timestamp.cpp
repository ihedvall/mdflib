/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf4timestamp.h"

#include <chrono>
#include <ctime>

using namespace std::chrono;

namespace mdf::detail {

void Mdf4Timestamp::GetBlockProperty(BlockPropertyList &dest) const {
  dest.emplace_back("ISO Time", GetTimeString());
  dest.emplace_back("TZ Offset [min]", std::to_string(tz_offset_));
  dest.emplace_back("DST Offset [min]", std::to_string(dst_offset_));

  std::ostringstream flags;
  flags << (flags_ & TimestampFlag::kLocalTimestamp ? "Local" : "UTC") << ","
        << (flags_ & TimestampFlag::kTimeOffsetValid ? "Offset" : "No Offset");

  dest.emplace_back("Time Flags", flags.str());
}

uint64_t Mdf4Timestamp::Read(std::streambuf& buffer) {
  file_position_ = GetFilePosition(buffer);
  uint64_t bytes = ReadNumber(buffer, time_);
  bytes += ReadNumber(buffer, tz_offset_);
  bytes += ReadNumber(buffer, dst_offset_);
  bytes += ReadNumber(buffer, flags_);
  return bytes;
}

uint64_t Mdf4Timestamp::Write(std::streambuf& buffer) {
  if (file_position_ <= 0) {
    file_position_ = GetFilePosition(buffer);
  } else {
    SetFilePosition(buffer, file_position_);
  }
  uint64_t bytes = WriteNumber(buffer, time_);
  bytes += WriteNumber(buffer, tz_offset_);
  bytes += WriteNumber(buffer, dst_offset_);
  bytes += WriteNumber(buffer, flags_);
  return bytes;
}

void Mdf4Timestamp::SetTime(uint64_t time) {
  time_ = time;

  // The offsets of the time itself, not of now. A summer time stamp that is
  // written in the winter shall keep its DST offset.
  const uint64_t time_s = time / 1'000'000'000;
  const auto seconds = static_cast<std::time_t>(time_s);
  std::tm local_tm{};
  std::tm utc_tm{};
  bool valid = static_cast<uint64_t>(seconds) == time_s;
  if (valid) {
    const std::tm* local = std::localtime(&seconds);
    if (local != nullptr) {
      local_tm = *local;
    }
    const std::tm* utc = std::gmtime(&seconds);
    if (utc != nullptr) {
      utc_tm = *utc;
    }
    valid = local != nullptr && utc != nullptr;
  }
  if (!valid) {
    // The time does not fit into time_t or cannot be converted. Use the
    // offsets of now as before.
    dst_offset_ = static_cast<int16_t>(MdfHelper::DstOffsetNs()/60'000'000'000);
    tz_offset_ = static_cast<int16_t>(MdfHelper::TimeZoneOffset() / 60);
    tz_offset_ -= dst_offset_;
  } else {
    int days = local_tm.tm_yday - utc_tm.tm_yday;
    if (days > 1) {
      days = -1;  // Local time is still in the previous year
    } else if (days < -1) {
      days = 1;  // Local time is already in the next year
    }
    const int offset_s = days * 24 * 3600 +
                         (local_tm.tm_hour - utc_tm.tm_hour) * 3600 +
                         (local_tm.tm_min - utc_tm.tm_min) * 60 +
                         (local_tm.tm_sec - utc_tm.tm_sec);
    dst_offset_ = local_tm.tm_isdst > 0 ? 60 : 0;  // As MdfHelper::DstOffsetNs()
    tz_offset_ = static_cast<int16_t>(offset_s / 60 - dst_offset_);
  }

  flags_ = TimestampFlag::kUtcTimestamp;
  flags_ |= TimestampFlag::kTimeOffsetValid;
}
void Mdf4Timestamp::SetTime(ITimestamp &timestamp) {
  time_ = timestamp.GetTimeNs();
  if (dynamic_cast<UtcTimestamp *>(&timestamp)) {
    flags_ = TimestampFlag::kUtcTimestamp;
    tz_offset_ = 0;
    dst_offset_ = 0;
  } else if (dynamic_cast<LocalTimestamp *>(&timestamp)) {
    flags_ = TimestampFlag::kLocalTimestamp;
    tz_offset_ = 0;
    dst_offset_ = 0;
  } else if (dynamic_cast<TimezoneTimestamp *>(&timestamp)) {
    flags_ = TimestampFlag::kTimeOffsetValid;
    tz_offset_ = timestamp.GetTimezoneMin();
    dst_offset_ = timestamp.GetDstMin();
  } else {
    throw std::runtime_error("Unknown timestamp type");
  }
}

Mdf4Timestamp::Mdf4Timestamp()
    : time_(0), tz_offset_(0), dst_offset_(0), flags_(0) {}

std::string Mdf4Timestamp::GetTimeString() const {
  const nanoseconds ns(time_);
  system_clock::time_point tp(duration_cast<system_clock::duration>(ns));
  minutes tz_offset(tz_offset_);
  minutes dst_offset(dst_offset_);
  tp += tz_offset + dst_offset;

  std::time_t time_t_tp = system_clock::to_time_t(tp);
  std::tm *tm_tp = std::gmtime(&time_t_tp);

  std::ostringstream date_time;
  date_time << std::put_time(tm_tp, "%Y-%m-%d %H:%M:%S");
  date_time << " + " << std::to_string(time_ % 1000'000'000) << " ns";

  if ((flags_ & (TimestampFlag::kUtcTimestamp | TimestampFlag::kTimeOffsetValid))
      == TimestampFlag::kUtcTimestamp) {
    date_time << " [UTC]";
  } else if ((flags_ & TimestampFlag::kTimeOffsetValid ) ==
                       TimestampFlag::kTimeOffsetValid) {
    date_time << " [GMT";
    if (tz_offset_ >= 0) {
      date_time << "+" << tz_offset_ / 60;
    }
    date_time << ", ";
    if (dst_offset_ > 0) {
      date_time << "DST+" << dst_offset_ / 60 << "h]";
    } else {
      date_time << "no DST]";
    }
  } else if ((flags_ & TimestampFlag::kLocalTimestamp) == TimestampFlag::kLocalTimestamp) {
    date_time << " [Local]";
  }

  return date_time.str();
}
uint64_t Mdf4Timestamp::GetTimeNs() const { return time_; }
uint16_t Mdf4Timestamp::GetTzOffsetMin() const { return tz_offset_; }
uint16_t Mdf4Timestamp::GetDstOffsetMin() const { return dst_offset_; }

timetype::MdfTimestampType Mdf4Timestamp::GetTimeType() const {
  if (flags_ & TimestampFlag::kTimeOffsetValid) {
    return timetype::kTimezoneTime;
  }
  if (flags_ & TimestampFlag::kLocalTimestamp) {
    return timetype::kLocalTime;
  }
  return timetype::kUtcTime;
}
}  // namespace mdf::detail