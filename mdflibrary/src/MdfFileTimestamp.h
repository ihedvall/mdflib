#pragma once
#include <cstdint>

namespace MdfLibrary {
public ref class MdfFileTimestamp {
public:
  MdfFileTimestamp(uint64_t time_ns, int16_t timezone_min, int16_t dst_min);
  property uint64_t TimeNs { uint64_t get(); }
  property int16_t TimezoneMin { int16_t get(); }
  property int16_t DstMin { int16_t get(); }

private:
  uint64_t time_ns_;
  int16_t timezone_min_;
  int16_t dst_min_;
};
}
