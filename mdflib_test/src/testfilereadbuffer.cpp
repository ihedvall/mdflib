/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "mdf/filereadbuffer.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"

namespace {

constexpr std::ios_base::openmode kRead =
    std::ios_base::in | std::ios_base::binary;

// Every test uses its own file name so that the tests can run in parallel
std::string TestFile(const std::string_view& name) {
  std::filesystem::path temp = std::filesystem::temp_directory_path();
  temp.append("test");
  std::filesystem::create_directories(temp);
  temp.append(name);
  return temp.string();
}

char Expected(int64_t position, int shift = 0) {
  return static_cast<char>((position + shift) % 251);
}

std::string MakeTestFile(const std::string_view& name, int64_t size,
                         int shift = 0) {
  const std::string filename = TestFile(name);
  std::ofstream file(filename, std::ios_base::out | std::ios_base::binary |
                                   std::ios_base::trunc);
  for (int64_t position = 0; position < size; ++position) {
    file.put(Expected(position, shift));
  }
  return filename;
}

}  // namespace

namespace mdf::test {

TEST(FileReadBuffer, SameAsFileBuffer) {
  // The same random steps on a std::filebuf and on the buffer. Most steps stay
  // near the last position, so the kept pages answer, are dropped and are read
  // again. Small reads go through the pages, large ones past them.
  for (const int64_t size : {0, 1, 4095, 4096, 4097, 100'000, 5'000'000}) {
    const std::string filename = MakeTestFile("filereadbuffer_same.bin", size);
    std::filebuf plain;
    FileReadBuffer buffer;
    ASSERT_NE(plain.open(filename, kRead), nullptr);
    ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
    std::vector<char> want(10'000);
    std::vector<char> got(10'000);

    // A large read leaves the file buffer below behind the block, so going
    // back to its start has to seek again
    ASSERT_EQ(buffer.sgetn(got.data(), 9'000), plain.sgetn(want.data(), 9'000));
    ASSERT_EQ(buffer.pubseekpos(0, std::ios_base::in),
              plain.pubseekpos(0, std::ios_base::in));
    ASSERT_EQ(buffer.sbumpc(), plain.sbumpc());

    std::mt19937 random(1);
    for (int step = 0; step < 5'000; ++step) {
      SCOPED_TRACE(testing::Message() << "size " << size << " step " << step);
      const auto here = static_cast<int64_t>(
          plain.pubseekoff(0, std::ios_base::cur, std::ios_base::in));
      const auto nearby =
          here + static_cast<int64_t>(random() % 40'000) - 20'000;
      const auto place = random() % 2 == 0 ? std::max<int64_t>(nearby, 0)
                                           : random() % (size + 100);
      switch (random() % 6) {
        case 0:
          ASSERT_EQ(buffer.pubseekpos(place, std::ios_base::in),
                    plain.pubseekpos(place, std::ios_base::in));
          break;
        case 1: {
          const auto back = -static_cast<int64_t>(random() % 100);
          ASSERT_EQ(
              buffer.pubseekoff(back, std::ios_base::end, std::ios_base::in),
              plain.pubseekoff(back, std::ios_base::end, std::ios_base::in));
          break;
        }
        case 2: {
          const auto ahead = static_cast<int64_t>(random() % 5'000);
          ASSERT_EQ(
              buffer.pubseekoff(ahead, std::ios_base::cur, std::ios_base::in),
              plain.pubseekoff(ahead, std::ios_base::cur, std::ios_base::in));
          break;
        }
        case 3:
          ASSERT_EQ(buffer.sbumpc(), plain.sbumpc());
          break;
        default: {
          const auto count = static_cast<std::streamsize>(
              random() % 2 == 0 ? random() % 100 : random() % 10'000);
          const auto read = plain.sgetn(want.data(), count);
          ASSERT_EQ(buffer.sgetn(got.data(), count), read);
          ASSERT_TRUE(
              std::equal(want.begin(), want.begin() + read, got.begin()));
          for (std::streamsize index = 0; index < read; ++index) {
            ASSERT_EQ(got[index], Expected(here + index));
          }
        }
      }
      ASSERT_EQ(buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in),
                plain.pubseekoff(0, std::ios_base::cur, std::ios_base::in));
    }
    // A seek before the start fails and keeps the position
    const auto last =
        buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in);
    EXPECT_EQ(buffer.pubseekpos(-1, std::ios_base::in), -1);
    EXPECT_EQ(buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in),
              last);
    // A position above 4 GB needs 64 bits
    const int64_t beyond = int64_t{5} << 30;
    EXPECT_EQ(buffer.pubseekpos(beyond, std::ios_base::in), beyond);
    EXPECT_EQ(buffer.sbumpc(), std::streambuf::traits_type::eof());
    EXPECT_EQ(buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in),
              beyond);
    buffer.File().close();
    plain.close();
    std::error_code error;
    std::filesystem::remove(filename, error);
  }
}

TEST(FileReadBuffer, ResetAndReopen) {
  const std::string filename =
      MakeTestFile("filereadbuffer_reopen.bin", 10'000);
  FileReadBuffer buffer;
  ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
  ASSERT_EQ(buffer.pubseekpos(4'097, std::ios_base::in), 4'097);
  EXPECT_EQ(buffer.sbumpc(), Expected(4'097));

  // Back to the start of the open file
  buffer.Reset();
  EXPECT_EQ(buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in), 0);
  EXPECT_EQ(buffer.sbumpc(), Expected(0));
  buffer.File().close();

  // The same name with other bytes. A page that was kept would answer with the
  // bytes of the first file.
  MakeTestFile("filereadbuffer_reopen.bin", 10'000, 7);
  buffer.Reset();
  ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
  EXPECT_EQ(buffer.sbumpc(), Expected(0, 7));
  buffer.File().close();

  // Opened at the end of the file, in a new buffer and after Reset
  FileReadBuffer fresh;
  ASSERT_NE(fresh.File().open(filename, kRead | std::ios_base::ate), nullptr);
  EXPECT_EQ(fresh.sbumpc(), Expected(0, 7));
  fresh.File().close();
  buffer.Reset();
  ASSERT_NE(buffer.File().open(filename, kRead | std::ios_base::ate), nullptr);
  EXPECT_EQ(buffer.pubseekpos(4'096, std::ios_base::in), 4'096);
  EXPECT_EQ(buffer.sbumpc(), Expected(4'096, 7));
  buffer.File().close();

  std::error_code error;
  std::filesystem::remove(filename, error);
}

TEST(FileReadBuffer, MdfReader) {
  // An application uses the buffer through the stream buffer interface of
  // MdfReader. The compressed data blocks are larger than a page.
  const std::string filename = TestFile("filereadbuffer_reader.mf4");
  constexpr size_t kSamples = 10'000;
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(filename));
  writer->CompressData(true);
  auto* channel_group =
      writer->Header()->CreateDataGroup()->CreateChannelGroup();
  auto* channel = channel_group->CreateChannel();
  channel->Name("Value");
  channel->Type(ChannelType::FixedLength);
  channel->DataType(ChannelDataType::FloatLe);
  channel->DataBytes(8);
  writer->PreTrigTime(0);
  ASSERT_TRUE(writer->InitMeasurement());
  uint64_t tick_time = 1'000'000'000;
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < kSamples; ++sample) {
    channel->SetChannelValue(static_cast<double>(sample));
    writer->SaveSample(*channel_group, tick_time);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  ASSERT_TRUE(writer->FinalizeMeasurement());

  auto buffer = std::make_shared<FileReadBuffer>();
  ASSERT_NE(buffer->File().open(filename, kRead), nullptr);
  MdfReader reader(buffer);
  ChannelObserverList observer_list;
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  for (auto* group : reader.GetFile()->Header()->DataGroups()) {
    for (const auto* sub_group : group->ChannelGroups()) {
      CreateChannelObserverForChannelGroup(*group, *sub_group, observer_list);
    }
    ASSERT_TRUE(reader.ReadData(*group));
  }
  reader.Close();
  EXPECT_TRUE(buffer->File().is_open());  // MdfReader leaves it open
  buffer->File().close();

  // The writer adds a time channel
  const auto observer =
      std::find_if(observer_list.cbegin(), observer_list.cend(),
                   [](const auto& item) { return item->Name() == "Value"; });
  ASSERT_NE(observer, observer_list.cend());
  ASSERT_EQ((*observer)->NofSamples(), kSamples);
  for (size_t sample = 0; sample < kSamples; ++sample) {
    double value = -1;
    ASSERT_TRUE((*observer)->GetChannelValue(sample, value));
    ASSERT_EQ(value, static_cast<double>(sample));
  }

  std::error_code error;
  std::filesystem::remove(filename, error);
}

}  // end namespace mdf::test
