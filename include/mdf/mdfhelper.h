/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace mdf {

class MdfHelper {
 public:

/** \brief Adds the time zone offset to the time stamp.
 *
 * Adds the time zone offset to the UTC nanoseconds since 1970.
 * @param [in] ns_since_1970
 * @return local time = system time + time zone offset
 */
  static uint64_t NanoSecToLocal(uint64_t ns_since_1970);

  /** \brief return the time zone offset in seconds.
 *
 * Returns the current used time zone offset in seconds
 * @return
 */
  static int64_t TimeZoneOffset();

/** \brief Converts from nanoseconds to CANopen 7 byte Date array.
 *
 * Converts from nanoseconds since 1970-01-01 to a 7 byte CANopen date array
 * This format is used in CANopen protocol and in ASAM MDF files.
 *
 * 7-byte CANopen date format:
 * \li uint16_t Milliseconds since last minute
 * \li uint8_t Minute (0..59)
 * \li uint8_t Hour (0..23)
 * \li uint8_t Day in month (1..31) + Day in week (1 = Monday..7 = Sunday)
 * \li uint8_t Month (1..12)
 * \li uint8_t Year (0..99)
 * @param [in] ns_since_1970 Nanoseconds since 1970-01-01
 * @return 7-byte CANopen date array
 */
  static std::vector<uint8_t> NsToCanOpenDateArray(uint64_t ns_since_1970);

  /** \brief Converts from nanoseconds to CANopen 6 byte Time array.
 *
 * Converts from nanoseconds since 1970-01-01 to a 6-byte CANopen time array
 * This format is used in CANopen protocol and in ASAM MDF files.
 *
 * 6-byte CANopen time format:
 * \li uint32_t Milliseconds since midnight
 * \li uint16_t Days since 1984

 * @param [in] ns_since_1970 Nanoseconds since 1970-01-01
 * @return 6-byte CANopen date array
 */
  static std::vector<uint8_t> NsToCanOpenTimeArray(uint64_t ns_since_1970);

  /** \brief Converts from a CANopen 7 byte Date array to nanoseconds since 1970.
 *
 * Converts from 7 byte CANopen date array to a uint64_t nanoseconds since 1970.
 * This format is used in CANopen protocol and in ASAM MDF files.
 *
 * 7-byte CANopen date format:
 * \li uint16_t Milliseconds since last minute
 * \li uint8_t Minute (0..59)
 * \li uint8_t Hour (0..23)
 * \li uint8_t Day in month (1..31) + Day in week (1 = Monday..7 = Sunday)
 * \li uint8_t Month (1..12)
 * \li uint8_t Year (0..99)
 * @param [in] buffer 7-byte CANopen date array
 * @return Nanoseconds since 1970-01-01
 */
  static uint64_t CanOpenDateArrayToNs(const std::vector<uint8_t> &buffer);

  /** \brief Converts from a CANopen 6 byte Time array to nanoseconds since 1970.
 *
 * Converts from nanoseconds since 1970-01-01 to a 6-byte CANopen time array
 * This format is used in CANopen protocol and in ASAM MDF files.
 *
 * 6-byte CANopen time format:
 * \li uint32_t Milliseconds since midnight
 * \li uint16_t Days since 1984

 * @param [in] buffer  6-byte CANopen time array
 * @return Nanoseconds since 1970-01-01
 */
  static uint64_t CanOpenTimeArrayToNs(const std::vector<uint8_t> &buffer);

/** \brief Converts ns since 1970 UTC to local date DD:MM:YYYY string
 *
 * Generates a local date string with the 'DD:MM:YYYY' format, from an UTC time stamp nanoseconds since
 * 1970-01-01 (midnight). The nanosecond  timestamp is commonly used when transfer a timestamp. Note that this may
 * not be used as epoch in different internal clocks.
 *
 * @param [in] ns_since_1970 Nanoseconds since 1970 UTC
 * @return Local date format 'DD:MM:YYYY'
 */
  static std::string NanoSecToDDMMYYYY(uint64_t ns_since_1970);

/** \brief Converts ns since 1970 UTC to local time HH:MM:SS string
 *
 * Generates a local time string with the 'HH:MM:ss' format, from an UTC time stamp nanoseconds since
 * 1970-01-01 (midnight). The nanosecond  timestamp is commonly used when transfer a timestamp. Note that this may
 * not be used as epoch in different internal clocks.
 *
 * @param [in] ns_since_1970 Nanoseconds since 1970 UTC
 * @return Local date format 'HH:MM:SS'
 */
  static std::string NanoSecToHHMMSS(uint64_t ns_since_1970);

};

} // end namespace