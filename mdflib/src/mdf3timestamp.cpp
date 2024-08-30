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
  local_timestamp_ = time + MdfHelper::GmtOffsetNs();
  utc_timestamp_ = time;
  dst_offset_ = static_cast<int16_t>(MdfHelper::DstOffsetNs() /
                                     timeunits::kNanosecondsPerHour);
  time_quality_ = 0;
  timer_id_ = "Local PC Reference Time";
}

void mdf::detail::Mdf3Timestamp::SetTime(mdf::ITimestamp& timestamp) {
  if (dynamic_cast<UtcTimeStamp*>(&timestamp)) {
    SetTime(timestamp.GetTimeNs());
    return;
  }

  if (auto local_time = dynamic_cast<LocalTimeStamp*>(&timestamp)) {
    date_ = MdfHelper::NanoSecUtcToDDMMYYYY(local_time->GetTimeNs());
    time_ = MdfHelper::NanoSecUtcToHHMMSS(local_time->GetTimeNs());
    local_timestamp_ = timestamp.GetTimeNs() - MdfHelper::DstOffsetNs();
    utc_timestamp_ = local_time->GetUtcTimeNs();
    dst_offset_ = static_cast<int16_t>(local_time->GetDstMin() / 60);
    time_quality_ = 0;
    timer_id_ = "Local PC Reference Time";
    return;
  }

  if (auto tz = dynamic_cast<TimezoneTimeStamp*>(&timestamp)) {
    date_ = MdfHelper::NanoSecTzToDDMMYYYY(
        timestamp.GetTimeNs(), tz->GetTimezoneMin(), tz->GetDstMin());
    time_ = MdfHelper::NanoSecTzToHHMMSS(timestamp.GetTimeNs(),
                                         tz->GetTimezoneMin(), tz->GetDstMin());
    local_timestamp_ = timestamp.GetTimeNs() +
                       tz->GetTimezoneMin() * timeunits::kNanosecondsPerMinute;
    utc_timestamp_ = tz->GetTimeNs();
    dst_offset_ = static_cast<int16_t>(timestamp.GetDstMin() / 60);
    time_quality_ = 0;
    timer_id_ = "Local PC Reference Time";
    return;
  }
}

uint64_t mdf::detail::Mdf3Timestamp::GetTime() const { return 0; }
