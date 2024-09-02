#pragma once
#include <cstdint>


namespace MdfNetHelper {
uint64_t GetUnixNanoTimestamp(System::DateTime time);

uint64_t GetLocalNanoTimestamp(System::DateTime time);
} // namespace MdfNetHelper
