#include "mdf/itimestamp.h"
#include "mdf/mdfhelper.h"

namespace mdf {
UtcTimestamp::UtcTimestamp(uint64_t utc_timestamp)
    : utc_timestamp_(utc_timestamp) {}

uint64_t UtcTimestamp::GetTimeNs() const { return utc_timestamp_; }
int16_t UtcTimestamp::GetDstMin() const { return 0; }
int16_t UtcTimestamp::GetTimezoneMin() const { return 0; }
uint64_t UtcTimestamp::GetUtcTimeNs() const { return utc_timestamp_; }

LocalTimestamp::LocalTimestamp(uint64_t local_timestamp)
    : local_timestamp_(local_timestamp) {
  timezone_offset_min_ = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                              timeunits::kNanosecondsPerMinute);
  dst_offset_min_ = static_cast<int16_t>(MdfHelper::DstOffsetNs() /
                                         timeunits::kNanosecondsPerMinute);
}

uint64_t LocalTimestamp::GetTimeNs() const { return local_timestamp_; }
int16_t LocalTimestamp::GetTimezoneMin() const { return timezone_offset_min_; }
int16_t LocalTimestamp::GetDstMin() const { return dst_offset_min_; }
uint64_t LocalTimestamp::GetUtcTimeNs() const {
  return local_timestamp_ -
         (MdfHelper::TimeZoneOffset() * timeunits::kNanosecondsPerSecond);
}

TimezoneTimestamp::TimezoneTimestamp(uint64_t utc_timestamp,
                                     int16_t timezone_offset_min,
                                     int16_t dst_offset_min)
    : utc_timestamp_(utc_timestamp),
      timezone_offset_min_(timezone_offset_min),
      dst_offset_min_(dst_offset_min) {}

uint64_t TimezoneTimestamp::GetTimeNs() const { return utc_timestamp_; }
int16_t TimezoneTimestamp::GetTimezoneMin() const {
  return timezone_offset_min_;
}
int16_t TimezoneTimestamp::GetDstMin() const { return dst_offset_min_; }
uint64_t TimezoneTimestamp::GetUtcTimeNs() const { return utc_timestamp_; }

}  // namespace mdf