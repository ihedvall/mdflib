/*
* Copyright 2026 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <optional>
#include <string>

#include "mdf/iheader.h"
#include "mdf/imdftimestamp.h"
#include "mdf/mdffactory.h"
#include "mdf/mdffile.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"

using namespace std::filesystem;

namespace {

void PutTimeZone(const char* tz) {
#ifdef _WIN32
  _putenv_s("TZ", tz != nullptr ? tz : "");
  _tzset();
#else
  if (tz != nullptr) {
    setenv("TZ", tz, 1);
  } else {
    unsetenv("TZ");
  }
  tzset();
#endif
}

// Sets TZ for one test and restores the previous value, also when an
// ASSERT ends the test early.
class TimeZoneGuard {
 public:
  explicit TimeZoneGuard(const char* tz) {
    if (const char* old = std::getenv("TZ"); old != nullptr) {
      old_ = old;
    }
    PutTimeZone(tz);
  }
  ~TimeZoneGuard() { PutTimeZone(old_ ? old_->c_str() : nullptr); }

 private:
  std::optional<std::string> old_;
};

}  // namespace

namespace mdf::test {

// The time zone and DST offsets in the HD block belong to the start time, not
// to the time the file is written. The Windows runtime always applies the US
// rules to EST5EDT, on POSIX they are set explicitly.
TEST(TestTimeOffset, StartTimeOffsets) {
#ifdef _WIN32
  TimeZoneGuard guard("EST5EDT");
#else
  TimeZoneGuard guard("EST5EDT,M3.2.0/2,M11.1.0/2");
#endif
  path dir = temp_directory_path() / "test" / "mdf" / "timeoffset";
  create_directories(dir);

  struct Case {
    const char* name;
    uint64_t start_time;  // ns since 1970 UTC
    uint16_t dst_offset_min;
  };
  const Case cases[] = {
      {"winter.mf4", 1'768'478'400'000'000'000, 0},   // 2026-01-15 12:00 UTC
      {"summer.mf4", 1'782'907'200'000'000'000, 60},  // 2026-07-01 12:00 UTC
      {"spring_before.mf4", 1'772'951'400'000'000'000, 0},  // 2026-03-08 06:30
      {"spring_after.mf4", 1'772'955'000'000'000'000, 60},  // 2026-03-08 07:30
      {"autumn_before.mf4", 1'793'511'000'000'000'000, 60},  // 2026-11-01 05:30
      {"autumn_after.mf4", 1'793'514'600'000'000'000, 0},    // 2026-11-01 06:30
  };
  for (const auto& test : cases) {
    const std::string filename = (dir / test.name).string();
    remove(filename);
    {
      auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
      ASSERT_TRUE(writer->Init(filename));
      ASSERT_TRUE(writer->InitMeasurement());
      writer->StartMeasurement(test.start_time);
      writer->StopMeasurement(test.start_time);
      ASSERT_TRUE(writer->FinalizeMeasurement());
    }
    MdfReader reader(filename);
    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadHeader());
    const auto* timestamp = reader.GetFile()->Header()->StartTimestamp();
    ASSERT_TRUE(timestamp != nullptr);
    EXPECT_EQ(timestamp->GetTimeNs(), test.start_time) << test.name;
    // The offsets are stored as int16_t, the getter returns them as uint16_t
    EXPECT_EQ(static_cast<int16_t>(timestamp->GetTzOffsetMin()), -300)
        << test.name;
    EXPECT_EQ(timestamp->GetDstOffsetMin(), test.dst_offset_min) << test.name;
  }
}

}  // namespace mdf::test
