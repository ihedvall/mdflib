#include "mdf3timestamp.h"

#include "mdf/mdfhelper.h"

void mdf::detail::Mdf3Timestamp::GetBlockProperty(
    mdf::detail::BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);
}

size_t mdf::detail::Mdf3Timestamp::Read(std::FILE* file) { return 0; }

size_t mdf::detail::Mdf3Timestamp::Write(std::FILE* file) {
  return MdfBlock::Write(file);
}
void mdf::detail::Mdf3Timestamp::SetTime(uint64_t time) {
  date_ = MdfHelper::NanoSecToDDMMYYYY(time);
  time_ = MdfHelper::NanoSecToHHMMSS(time);
  local_timestamp_ = MdfHelper::NanoSecToLocal(time);
  utc_timestamp_ =
      local_timestamp_ - (MdfHelper::TimeZoneOffset() * 1'000'000'000LL);
  dst_offset_ = static_cast<int16_t>(MdfHelper::TimeZoneOffset() / 3600);
  time_quality_ = 0;
  timer_id_ = "Local PC Reference Time";
}

void mdf::detail::Mdf3Timestamp::SetTime(mdf::ITimestamp& timestamp) {
  if (dynamic_cast<UtcTimeStamp*>(&timestamp)) {
    SetTime(timestamp.GetTime());
  }

  if (dynamic_cast<LocalTimeStamp*>(&timestamp)) {
    date_ = MdfHelper::NanoSecUtcToDDMMYYYY(timestamp.GetTime());
    time_ = MdfHelper::NanoSecUtcToHHMMSS(timestamp.GetTime());
    local_timestamp_ = timestamp.GetTime();
    utc_timestamp_ =
        local_timestamp_ - (MdfHelper::TimeZoneOffset() * 1'000'000'000LL);
    dst_offset_ = static_cast<int16_t>(MdfHelper::TimeZoneOffset() / 3600);
    time_quality_ = 0;
    timer_id_ = "Local PC Reference Time";
  }

  if (auto tz = dynamic_cast<TimezoneTimeStamp*>(&timestamp)) {
    date_ = MdfHelper::NanoSecTzToDDMMYYYY(timestamp.GetTime(),
                                           tz->GetTimezone(), tz->GetDst());
    time_ = MdfHelper::NanoSecTzToHHMMSS(timestamp.GetTime(), tz->GetTimezone(),
                                         tz->GetDst());
    local_timestamp_ = MdfHelper::NanoSecToTimezone(
        timestamp.GetTime(), tz->GetTimezone(), tz->GetDst());
    utc_timestamp_ = tz->GetTime();
    dst_offset_ = static_cast<int16_t>(timestamp.GetDst() / 60);
    time_quality_ = 0;
    timer_id_ = "Local PC Reference Time";
  }
}

uint64_t mdf::detail::Mdf3Timestamp::GetTime() const { return 0; }
