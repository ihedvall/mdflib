#include "TimeUtil.h"
namespace util::time {
uint64_t TimeStampToNs(TimeStamp timestamp) {
  // 1.na since midnight
  // 2 sec since 1970
  // 3 Add them
  const auto ns_midnight = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               timestamp.time_since_epoch()) %
                           1000'000'000;
  const auto sec_1970 = std::chrono::system_clock::to_time_t(timestamp);
  uint64_t ns = sec_1970;
  ns *= 1'000'000'000;
  ns += ns_midnight.count();
  return ns;
}
std::string GetLocalTimestampWithMs(
    std::chrono::time_point<std::chrono::system_clock> timestamp) {
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      timestamp.time_since_epoch()) %
                  1000;
  const auto timer = std::chrono::system_clock::to_time_t(timestamp);
  const struct tm* bt = localtime(&timer);

  std::ostringstream text;
  text << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0')
       << std::setw(3) << ms.count();
  return text.str();
}
}  // namespace util::time