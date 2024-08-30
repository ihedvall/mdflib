#pragma once

#include "mdf/itimestamp.h"
#include "mdfblock.h"

namespace mdf {
/**
 * \brief Interface for MDF timestamp handling.
 */
class IMdfTimeStamp {
 public:
  /**
   * \brief Set the time in nanoseconds.
   * \param time The time in nanoseconds.
   */
  virtual void SetTime(uint64_t time) = 0;
  /**
   * \brief Set the time using an ITimestamp object.
   * \param timestamp An ITimestamp object representing the time.
   */
  virtual void SetTime(ITimestamp &timestamp) = 0;
  /**
   * \brief Get the time in nanoseconds.
   * \return The time in nanoseconds.
   */
  [[nodiscard]] virtual uint64_t GetTimeNs() const = 0;
  /**
   * \brief Get the timezone offset in minutes.
   * \return The timezone offset in minutes.
   */
  [[nodiscard]] virtual uint16_t GetTzOffsetMin() const = 0;
  /**
   * \brief Get the daylight saving time offset in minutes.
   * \return The daylight saving time offset in minutes.
   */
  [[nodiscard]] virtual uint16_t GetDstOffsetMin() const = 0;
};
}  // namespace mdf::detail