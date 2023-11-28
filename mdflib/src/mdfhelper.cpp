/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/mdfhelper.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <codecvt>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "littlebuffer.h"

namespace {

void LTrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

void RTrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

}  // namespace

namespace mdf {

uint64_t MdfHelper::NanoSecToLocal(uint64_t ns_since_1970) {
  const auto utc_offset = TimeZoneOffset();
  return ns_since_1970 + (utc_offset * 1'000'000'000);
}

int64_t MdfHelper::TimeZoneOffset() {
  const auto system_time = std::time(nullptr);
  struct tm *utc = gmtime(&system_time);
  utc->tm_isdst = -1;
  const auto local_time = mktime(utc);
  return system_time - local_time;
}

std::string MdfHelper::NsToLocalIsoTime(uint64_t ns_since_1970) {
  const auto ms_sec = (ns_since_1970 / 1'000'000) % 1'000;
  const auto system_time =
      static_cast<std::time_t>(ns_since_1970 / 1'000'000'000);
  struct tm *bt = std::localtime(&system_time);
  std::ostringstream text;
  text << std::put_time(bt, "%Y-%m-%d %H:%M:%S");
  if (ms_sec > 0) {
    text << '.' << std::setfill('0') << std::setw(3) << ms_sec;
  }
  return text.str();
}

std::vector<uint8_t> MdfHelper::NsToCanOpenDateArray(uint64_t ns_since_1970) {
  std::vector<uint8_t> date_array(7, 0);
  const auto system_time = static_cast<time_t>(
      ns_since_1970 / 1'000'000'000);  // Convert to sec since 1970
  const auto ms_min = static_cast<uint16_t>((ns_since_1970 / 1'000'000) %
                                            60'000);  // ms since minute shift

  const LittleBuffer data(ms_min);
  memcpy(date_array.data(), data.data(), data.size());

  const struct tm *bt = localtime(&system_time);
  date_array[2] = bt->tm_min;
  date_array[3] = bt->tm_hour;
  if (bt->tm_isdst) {
    date_array[3] |= 0x80;
  }
  date_array[4] = bt->tm_mday;
  uint8_t day_in_week = bt->tm_wday == 0 ? 7 : bt->tm_wday;
  date_array[4] |= (day_in_week << 5);
  date_array[5] = 1 + bt->tm_mon;
  date_array[6] = bt->tm_year % 100;
  return date_array;
}

std::vector<uint8_t> MdfHelper::NsToCanOpenTimeArray(uint64_t ns_since_1970) {
  // Time Array Format (6 byte, little endian):
  // uint32_t ms since midnight
  // uint16_t days since 1984-01-01
  std::vector<uint8_t> time_array(6, 0);

  // Calculate second to 1984-01-01
  struct tm bt {};
  bt.tm_year = 84;
  bt.tm_mon = 0;
  bt.tm_mday = 1;
  const auto sec_1984 = mktime(&bt);

  uint64_t ms_since_1984 = ns_since_1970 / 1'000'000;
  ms_since_1984 -= static_cast<uint64_t>(sec_1984) * 1'000;

  const auto days_since_1984 =
      static_cast<uint16_t>(ms_since_1984 / (24 * 3'600'000));

  const LittleBuffer milli_sec(
      static_cast<uint32_t>(ms_since_1984 % (24 * 3'600'000)));
  memcpy(time_array.data(), milli_sec.data(), 4);

  const LittleBuffer days(days_since_1984);
  memcpy(time_array.data() + 4, days.data(), 2);

  return time_array;
}

uint64_t MdfHelper::CanOpenTimeArrayToNs(const std::vector<uint8_t> &buffer) {
  const LittleBuffer<uint32_t> ms(buffer, 0);    // Milliseconds since midnight
  const LittleBuffer<uint16_t> days(buffer, 4);  // Days since 1984-01-01

  struct tm time {};
  time.tm_mday = 1;   // 1..31
  time.tm_mon = 0;    // Note 0..11
  time.tm_year = 84;  // Years since 1900

  const auto sec_1984 = std::mktime(&time);  // Hope that epoch is 1970
  uint64_t dest = static_cast<uint64_t>(sec_1984) * 1'000'000'000;
  dest += static_cast<uint64_t>(days.value()) * 24 * 3'600'000'000'000;
  dest += static_cast<uint64_t>(ms.value()) * 1'000'000;
  return dest;
}

uint64_t MdfHelper::CanOpenDateArrayToNs(const std::vector<uint8_t> &buffer) {
  const LittleBuffer<uint16_t> ms_min(buffer, 0);  // ms since last minute
  const uint8_t min = buffer[2] & 0x3F;
  const uint8_t hour = buffer[3] & 0x1F;
  const bool dst = (0x80 & buffer[3]) != 0;
  const uint8_t day_in_week = (buffer[4] & 0xE0) >> 5;  // 1 Monday..7 = Sunday
  const uint8_t day = buffer[4] & 0x1F;                 // 1..31

  const uint8_t month = buffer[5] & 0x3F;  // 1-12
  const uint8_t year = buffer[6] & 0x7F;   // 0.99

  // Convert to milliseconds since 1970

  uint64_t dest = static_cast<uint64_t>(ms_min.value()) * 1'000'000;

  struct tm time {};
  time.tm_sec = 0;
  time.tm_min = min;
  time.tm_hour = hour;
  time.tm_mday = day;                             // 1..31
  time.tm_mon = month - 1;                        // Note 0..11
  time.tm_year = year >= 70 ? year : 100 + year;  // Years since 1900
  time.tm_wday =
      day_in_week == 7 ? 0 : day_in_week;  // 0 = Sunday..6 = Saturday. Actually
                                           // ignored by mktime()
  time.tm_yday = 0;                        // Ignored by mktime()
  time.tm_isdst = dst ? 1 : 0;

  std::time_t sec_1970 = std::mktime(&time);  // Hope that epoch is 1970
  dest += static_cast<uint64_t>(sec_1970) * 1'000'000'000;
  return dest;
}

std::string MdfHelper::NanoSecToDDMMYYYY(uint64_t ns_since_1970) {
  auto system_time = static_cast<time_t>(ns_since_1970 / 1'000'000'000ULL);
  const struct tm *bt = localtime(&system_time);
  std::ostringstream s;
  s << std::put_time(bt, "%d:%m:%Y");
  return s.str();
}

std::string MdfHelper::NanoSecToHHMMSS(uint64_t ns_since_1970) {
  const auto system_time =
      static_cast<time_t>(ns_since_1970 / 1'000'000'000ULL);
  const struct tm *bt = localtime(&system_time);
  std::ostringstream s;
  s << std::put_time(bt, "%H:%M:%S");
  return s.str();
}

void MdfHelper::Trim(std::string &text) {
  LTrim(text);
  RTrim(text);
}

std::string MdfHelper::FormatDouble(double value, uint8_t decimals, bool fixed,
                                    const std::string &unit) {
  // Maximize it to 20 decimals
  if (decimals > 20) {
    decimals = 20;
  }

  // If no decimals, the fixed must be false
  if (decimals == 0) {
    fixed = false;
  }

  std::string text;
  auto value_int = static_cast<int64_t>(value);
  if (value == value_int && !fixed) {
    // If the value actually is an integer then just show it as an integer
    text = std::to_string(value_int);
  } else if (decimals == 0) {
    // Fixed round of float
    value_int = static_cast<int64_t>(std::floor(value + 0.5));
    text = std::to_string(value_int);
  } else {
    char format[20]{};
    snprintf(format, sizeof(format), "%%.%df", static_cast<int>(decimals));
    char temp[200]{};
    snprintf(temp, sizeof(temp), format, value);
    text = temp;
  }

  // If the value is producing to many digits, then convert it to a string with
  // exponent
  const auto size = text.size();
  if (size > static_cast<size_t>(6 + decimals)) {
    char temp[200]{};
    snprintf(temp, sizeof(temp), "%G", value);
    text = temp;
  }

  // fill or delete trailing '0' but not if using exponent
  const bool have_decimal = text.find('.') != std::string::npos &&
                            text.find('E') == std::string::npos;
  if (!fixed && have_decimal) {
    // Remove trailing zeros
    // Check that it is a decimal point in the string
    while (text.back() == '0') {
      text.pop_back();
    }
  } else if (fixed && have_decimal) {
    const size_t dec_pos = text.find('.');
    const std::string dec = text.substr(dec_pos + 1);
    for (size_t nof_dec = dec.size(); nof_dec < decimals; ++nof_dec) {
      text.push_back('0');
    }
  }

  // We don't want to display '22.' instead of 22
  if (text.back() == '.') {
    text.pop_back();
  }

  if (!unit.empty()) {
    text += " ";
    text += unit;
  }
  return text;
}

uint64_t MdfHelper::NowNs() {
  const auto now = std::chrono::system_clock::now();
  const auto ns_midnight = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               now.time_since_epoch()) %
                           1000'000'000;
  const auto sec_1970 = std::chrono::system_clock::to_time_t(now);
  uint64_t ns = sec_1970;
  ns *= 1'000'000'000;
  ns += ns_midnight.count();
  return ns;
}

std::string MdfHelper::Latin1ToUtf8(const std::string &latin1) {
  std::ostringstream utf8;
  for (const auto in_char : latin1) {
    const auto ch = static_cast<const uint8_t>(in_char);
    if (ch < 0x80) {
      utf8 << ch;
    } else {
      utf8 << (0xC0 | ch >> 6);
      utf8 << (0x80 | (ch & 0x3f));
    }
  }
  return utf8.str();
}

std::string MdfHelper::Utf16ToUtf8(const std::wstring &utf16) {
  std::u16string u16str(utf16.begin(), utf16.end());
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return  convert.to_bytes(u16str);
}

std::wstring MdfHelper::Utf8ToUtf16(const std::string &utf8) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  const auto utf16 = convert.from_bytes(utf8);
  return {utf16.begin(), utf16.end()};
}

bool MdfHelper::ComputerUseLittleEndian() {
  constexpr int num = 1;
  return *(reinterpret_cast<const char *>(&num)) == 1;
}

std::vector<uint8_t> MdfHelper::TextToByteArray(const std::string &text) {
  std::vector<uint8_t> byte_array(text.size());
  for (size_t index = 0; index < text.size(); ++index) {
    byte_array[index] = static_cast<const uint8_t>(text[index]);
  }
  return byte_array;
}

}  // namespace mdf