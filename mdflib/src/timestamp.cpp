#include "mdf/itimestamp.h"
#include "mdf/mdfhelper.h"

namespace mdf {
UtcTimeStamp::UtcTimeStamp(uint64_t utc_timestamp)
    : utc_timestamp_(utc_timestamp) {}

uint64_t UtcTimeStamp::GetTimeNs() const { return utc_timestamp_; }
int16_t UtcTimeStamp::GetDstMin() const { return 0; }
int16_t UtcTimeStamp::GetTimezoneMin() const { return 0; }
uint64_t UtcTimeStamp::GetUtcTimeNs() const { return utc_timestamp_; }

LocalTimeStamp::LocalTimeStamp(uint64_t local_timestamp)
    : local_timestamp_(local_timestamp) {
  timezone_offset_min_ = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                              timeunits::kNanosecondsPerMinute);
  dst_offset_min_ = static_cast<int16_t>(MdfHelper::DstOffsetNs() /
                                         timeunits::kNanosecondsPerMinute);
}

uint64_t LocalTimeStamp::GetTimeNs() const { return local_timestamp_; }
int16_t LocalTimeStamp::GetTimezoneMin() const { return timezone_offset_min_; }
int16_t LocalTimeStamp::GetDstMin() const { return dst_offset_min_; }
uint64_t LocalTimeStamp::GetUtcTimeNs() const {
  return local_timestamp_ -
         (MdfHelper::TimeZoneOffset() * timeunits::kNanosecondsPerSecond);
}

TimezoneTimeStamp::TimezoneTimeStamp(uint64_t utc_timestamp,
                                     int16_t timezone_offset_min,
                                     int16_t dst_offset_min)
    : utc_timestamp_(utc_timestamp),
      timezone_offset_min_(timezone_offset_min),
      dst_offset_min_(dst_offset_min) {}

uint64_t TimezoneTimeStamp::GetTimeNs() const { return utc_timestamp_; }
int16_t TimezoneTimeStamp::GetTimezoneMin() const {
  return timezone_offset_min_;
}
int16_t TimezoneTimeStamp::GetDstMin() const { return dst_offset_min_; }
uint64_t TimezoneTimeStamp::GetUtcTimeNs() const { return utc_timestamp_; }

}  // namespace mdf