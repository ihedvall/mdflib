#include "MdfFileTimestamp.h"


namespace MdfLibrary {
MdfFileTimestamp::MdfFileTimestamp(const uint64_t time_ns,
                                   const int16_t timezone_min,
                                   const int16_t dst_min) {
  time_ns_ = time_ns;
  timezone_min_ = timezone_min;
  dst_min_ = dst_min;
}

uint64_t MdfFileTimestamp::TimeNs::get() {
  return time_ns_;
}

int16_t MdfFileTimestamp::TimezoneMin::get() {
  return timezone_min_;
}

int16_t MdfFileTimestamp::DstMin::get() {
  return dst_min_;
}
}
