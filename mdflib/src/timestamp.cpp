#include "mdf/itimestamp.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace mdf {
UtcTimeStamp::UtcTimeStamp(uint64_t utc_timestamp)
    : utc_timestamp_(utc_timestamp) {}

uint64_t UtcTimeStamp::GetTime() const { return utc_timestamp_; }
int16_t UtcTimeStamp::GetDst() const { return 0; }
int16_t UtcTimeStamp::GetTimezone() const { return 0; }

LocalTimeStamp::LocalTimeStamp(uint64_t local_timestamp)
    : local_timestamp_(local_timestamp) {}

uint64_t LocalTimeStamp::GetTime() const { return local_timestamp_; }
int16_t LocalTimeStamp::GetTimezone() const { return 0; }
int16_t LocalTimeStamp::GetDst() const { return 0; }

TimezoneTimeStamp::TimezoneTimeStamp(uint64_t utc_timestamp,
                                     int16_t timezone_offset_min,
                                     int16_t dst_offset_min)
    : utc_timestamp_(utc_timestamp),
      timezone_offset_min_(timezone_offset_min),
      dst_offset_min_(dst_offset_min) {}

uint64_t TimezoneTimeStamp::GetTime() const { return utc_timestamp_; }
int16_t TimezoneTimeStamp::GetTimezone() const { return timezone_offset_min_; }
int16_t TimezoneTimeStamp::GetDst() const { return dst_offset_min_; }

}  // namespace mdf