#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
namespace util::time {

/** \typedef SystemClock
 * \brief Convenience type definition for std::chrono::system_clock;
 */
using SystemClock = std::chrono::system_clock;

/** \typedef TimeStamp
 * \brief Convenience type definition for
 * std::chrono::time_point<std::chrono::system_time>;
 */
using TimeStamp = std::chrono::time_point<SystemClock>;

uint64_t TimeStampToNs(TimeStamp timestamp = SystemClock::now());
std::string GetLocalTimestampWithMs(
    std::chrono::time_point<std::chrono::system_clock> timestamp =
        SystemClock::now());
}  // namespace util::time