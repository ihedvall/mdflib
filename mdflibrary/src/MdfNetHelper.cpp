#include "MdfNetHelper.h"

uint64_t MdfNetHelper::GetUnixNanoTimestamp(System::DateTime time) {
  auto epoch =
      System::DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);

  System::TimeSpan elapsed_time = time.ToUniversalTime().Subtract(epoch);

  const uint64_t nano_seconds =
      static_cast<uint64_t>(elapsed_time.TotalSeconds) * 1000000000ULL +
      static_cast<uint64_t>(elapsed_time.Milliseconds) * 1000000ULL;

  return nano_seconds;
}

uint64_t MdfNetHelper::GetLocalNanoTimestamp(System::DateTime time) {
  auto epoch =
      System::DateTime(1970, 1, 1, 0, 0, 0, System::DateTimeKind::Utc);
  System::DateTime local_time = time.ToLocalTime();
  System::TimeSpan elapsed_time = local_time.Subtract(epoch);
  const uint64_t nano_seconds =
      static_cast<uint64_t>(elapsed_time.TotalSeconds) * 1000000000ULL +
      static_cast<uint64_t>(elapsed_time.Milliseconds) * 1000000ULL;

  return nano_seconds;
}
