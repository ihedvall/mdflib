/*
* Copyright 2026 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include "mdf/mdfhelper.h"
#include <boost/locale.hpp>
#include <filesystem>

using namespace std::filesystem;
using namespace boost::locale;
namespace mdf::test {

TEST(MdfHelperTest, NanoSecToLocal) {
  const uint64_t now = MdfHelper::NowNs();
  EXPECT_GT(now, 0);
  const uint64_t local = MdfHelper::NanoSecToLocal(now);
  EXPECT_GT(local, 0);
  const uint64_t ref_time = now + (MdfHelper::TimeZoneOffset() * 1'000'000'000);
  EXPECT_EQ(local, ref_time);
}

TEST(MdfHelperTest, TimeZoneOffset) {
  int64_t tz_offset = MdfHelper::TimeZoneOffset() / 60;
  std::cout << "Timezone Offset (min): " << tz_offset  << std::endl;
}

TEST(MdfHelperTest, GmtOffsetNs) {
  int64_t dst_offset = MdfHelper::GmtOffsetNs() / 60'000'000'000;
  std::cout << "DST Offset (min): " << dst_offset  << std::endl;
}

TEST(MdfHelperTest, NsToLocalIsoTime) {
  const uint64_t now = MdfHelper::NowNs();
  const std::string iso_time = MdfHelper::NsToLocalIsoTime(now);
  std::cout << "Now: " << iso_time << std::endl;
  EXPECT_FALSE(iso_time.empty());
  EXPECT_EQ(iso_time.size(), 23);
}

TEST(MdfHelperTest, NsToCanOpenDateArray) {
  const uint64_t now = MdfHelper::NowNs();
  const auto can_dt = MdfHelper::NsToCanOpenDateArray(now);
  EXPECT_EQ(can_dt.size(), 7);

  constexpr uint64_t ns1970 = 0;
  const auto ns1970_dt = MdfHelper::NsToCanOpenDateArray(ns1970);
  ASSERT_EQ(ns1970_dt.size(), 7);
  EXPECT_EQ(ns1970_dt[0], 0);
  EXPECT_EQ(ns1970_dt[1], 0);
  // EXPECT_EQ(ns1970_dt[2], 0); GMT 4.5 h in idia
  // EXPECT_EQ(ns1970_dt[3] & 0x1F, 0); local time
  EXPECT_EQ(ns1970_dt[4] & 0x1F, 1);
  EXPECT_EQ(ns1970_dt[4] >> 5, 4); // Thursday
  EXPECT_EQ(ns1970_dt[5], 1);
  EXPECT_EQ(ns1970_dt[6], 70);
}

TEST(MdfHelperTest, NsToCanOpenTimeArray) {
  const uint64_t now = MdfHelper::NowNs();
  const auto can_dt = MdfHelper::NsToCanOpenTimeArray(now);
  EXPECT_EQ(can_dt.size(), 6);
}

TEST(MdfHelperTest, CanOpenDateArrayToNs) {
  uint64_t now = MdfHelper::NowNs();
  now /= 1'000'000; // Normalize to ms
  now *= 1'000'000;
  const auto can_dt = MdfHelper::NsToCanOpenDateArray(now);
  EXPECT_EQ(can_dt.size(), 7);
  const uint64_t ref_ns = MdfHelper::CanOpenDateArrayToNs(can_dt);
  EXPECT_EQ(ref_ns, now);
}

TEST(MdfHelperTest, CanOpenTimeArrayToNs) {
  uint64_t now = MdfHelper::NowNs();
  now /= 1'000'000; // Normalize to ms
  now *= 1'000'000;
  const auto can_dt = MdfHelper::NsToCanOpenTimeArray(now);
  EXPECT_EQ(can_dt.size(), 6);
  const uint64_t ref_ns = MdfHelper::CanOpenTimeArrayToNs(can_dt);
  EXPECT_EQ(ref_ns, now);
}

TEST(MdfHelperTest, NanoSeToDDMMYYYY) {
  uint64_t now = MdfHelper::NowNs();
  const std::string date = MdfHelper::NanoSecToDDMMYYYY(now);
  EXPECT_EQ(date.size(), 10);
  std::cout << "Date: " << date << std::endl;
}

TEST(MdfHelperTest, NanoSecToHHMMSS) {
  uint64_t now = MdfHelper::NowNs();
  const std::string time = MdfHelper::NanoSecToHHMMSS(now);
  EXPECT_EQ(time.size(), 8);
  std::cout << "Time: " << time << std::endl;
}

TEST(MdfHelperTest, NanoSecUtcToHHMMSS) {
  uint64_t now = MdfHelper::NowNs();
  const std::string time = MdfHelper::NanoSecUtcToHHMMSS(now);
  EXPECT_EQ(time.size(), 8);
  std::cout << "UTC Time: " << time << std::endl;
}

TEST(MdfHelperTest, NanoSecUtcToDDMMYYYY) {
  const uint64_t now = MdfHelper::NowNs();
  const std::string date = MdfHelper::NanoSecUtcToDDMMYYYY(now);
  EXPECT_EQ(date.size(), 10);
  std::cout << "UTC Date: " << date << std::endl;
}

TEST(MdfHelperTest, NanoSecTzToHHMMSS) {
  const uint64_t now = MdfHelper::NowNs();
  const auto dst_offset = static_cast<int16_t>(MdfHelper::DstOffsetNs()
    / 60'000'000'000);
  auto tz_offset = static_cast<int16_t>(MdfHelper::TimeZoneOffset() / 60);
  tz_offset = static_cast<int16_t>(tz_offset - dst_offset);


  const std::string time = MdfHelper::NanoSecTzToHHMMSS(now, tz_offset,
    dst_offset );
  EXPECT_EQ(time.size(), 8);
  std::cout << "TZ Time: " << time
    << ", TZ (min): " << tz_offset
    << ". DST (min): " << dst_offset << std::endl;
}

TEST(MdfHelperTest, NanoSecTzToDDMMYYYY) {
  const uint64_t now = MdfHelper::NowNs();
  const auto dst_offset = static_cast<int16_t>(MdfHelper::DstOffsetNs()
    / 60'000'000'000);
  auto tz_offset = static_cast<int16_t>(MdfHelper::TimeZoneOffset() / 60);
  tz_offset = static_cast<int16_t>(tz_offset - dst_offset);
  const std::string date = MdfHelper::NanoSecTzToDDMMYYYY(now,
    tz_offset,dst_offset
    );
  EXPECT_EQ(date.size(), 10);
  std::cout << "TZ Date: " << date
    << ", TZ (min): " << tz_offset
    << ". DST (min): " << dst_offset << std::endl;
}

TEST(MdfHelperTest, Trim) {
  std::string trim_text = " \n Hello World  \t\r\n";
  constexpr std::string_view kTrimmed = "Hello World";
  MdfHelper::Trim(trim_text);
  EXPECT_EQ(trim_text, kTrimmed);
}

TEST(MdfHelperTest, FormatDouble) {
  constexpr double ref_value1 = -1.0/3.0;
  const std::string dec_text = MdfHelper::FormatDouble(ref_value1,
    2, false, "s");
  EXPECT_EQ(dec_text, "-0.33 s");

  constexpr double ref_value2 = 1.3;
  const std::string not_fixed_text = MdfHelper::FormatDouble(ref_value2,
    2, false, "s");
  EXPECT_EQ(not_fixed_text, "1.3 s");
  const std::string fixed_text = MdfHelper::FormatDouble(ref_value2,
    2, true, "s");
  EXPECT_EQ(fixed_text, "1.30 s");
}

TEST(MdfHelperTest, Latin1ToUtf8) {
  constexpr std::string_view latin1_text = "ÅÄÖ";
  const std::string ref_utf8 = conv::to_utf<char>(
    std::string(latin1_text), "Latin1");
  const std::string utf8_text = MdfHelper::Latin1ToUtf8(std::string(latin1_text));
  EXPECT_EQ(utf8_text, ref_utf8);
  EXPECT_GT(utf8_text.size(), latin1_text.size());

  std::cout << "LATIN1 to UTF8" << std::endl;
  std::cout << "Latin1 Text: " << latin1_text << std::endl;
  std::cout << "UTF8 Text: " << utf8_text << std::endl;
  std::cout << std::endl;
}

TEST(MdfHelperTest, Utf16ToUtf8) {
  constexpr std::wstring_view utf16_text = L"ÅÄÖ";
  const std::string ref_utf8 = conv::utf_to_utf<char>(
    std::wstring(utf16_text) );
  const std::string utf8_text = MdfHelper::Utf16ToUtf8(std::wstring(utf16_text));
  EXPECT_GT(utf8_text.size(), 0);
  EXPECT_EQ(utf8_text, ref_utf8);

  std::cout << "UTF16 to UTF8" << std::endl;
  std::cout << "UTF8 Text: " << utf8_text << std::endl;
  std::cout << std::endl;
}

TEST(MdfHelperTest, Utf8ToUtf16) {
  constexpr std::string_view latin1_text = "ÅÄÖ";
  const std::string ref_utf8 = conv::to_utf<char>(std::string(latin1_text),
    "Latin1");
  const std::wstring utf16_text = MdfHelper::Utf8ToUtf16(ref_utf8);
  const std::wstring ref_utf16 = conv::utf_to_utf<wchar_t>(ref_utf8);
  EXPECT_GT(utf16_text.size(), 0);
  EXPECT_EQ(utf16_text, ref_utf16);
}

TEST(MdfHelperTest, ComputerUseLittleEndian) {
  EXPECT_TRUE(MdfHelper::ComputerUseLittleEndian());
}

TEST(MdfHelperTest, TextToByteArray) {
  const std::vector<uint8_t> ref_byte_array {
    static_cast<uint8_t>('A'),
    static_cast<uint8_t>('B'),
    static_cast<uint8_t>('C'),
  };
  constexpr std::string_view text = "ABC";
  std::vector<uint8_t> byte_array =
    MdfHelper::TextToByteArray(std::string(text));
  EXPECT_EQ(byte_array, ref_byte_array);
}

TEST(MdfHelperTest, UnsignedToRaw) {
  std::array<uint8_t, 8> raw_array = {};
  {
    raw_array.fill(0x00);
    constexpr uint8_t ref_value = 0x12;
    MdfHelper::UnsignedToRaw(true, 0, 8,
      ref_value, raw_array.data());
    EXPECT_EQ(raw_array[0], ref_value);

    raw_array.fill(0x00);
    MdfHelper::UnsignedToRaw(false, 7, 8,
      ref_value, raw_array.data());
    EXPECT_EQ(raw_array[0], ref_value);

    raw_array.fill(0x00);
    MdfHelper::UnsignedToRaw(false, 8+7, 8,
      ref_value, raw_array.data());
    EXPECT_EQ(raw_array[0], 0x00);
    EXPECT_EQ(raw_array[1], ref_value);
  }
  {
    raw_array.fill(0x00);
    constexpr uint16_t ref_value = 0x1234;
    MdfHelper::UnsignedToRaw(true, 0, 16,
      ref_value, raw_array.data());
    EXPECT_EQ(raw_array[0], 0x34);
    EXPECT_EQ(raw_array[1], 0x12);

    raw_array.fill(0x00);
    MdfHelper::UnsignedToRaw(false, 7, 16,
      ref_value, raw_array.data());
    EXPECT_EQ(raw_array[0], 0x12);
    EXPECT_EQ(raw_array[1], 0x34);
  }

}

}
