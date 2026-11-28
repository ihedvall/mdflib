/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "testread.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <iostream>
#include <iterator>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdflogstream.h"

#include "mdf3file.h"
#include "mdf4file.h"

#include <util/logconfig.h>
#include <util/logstream.h>
#include <util/stringutil.h>

using namespace std::filesystem;
using namespace util::string;
using namespace mdf;
using namespace mdf::detail;
using namespace util::log;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace boost::iostreams;

namespace {
const std::string mdf_source_dir =
    "k:/test/mdf";  // Where all source MDF files exist

constexpr std::string_view  kLargeLocalSsdFile =
    "c:/temp/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";
constexpr std::string_view  kLargeLocalDriveFile =
    "o:/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";
constexpr std::string_view  kLargeNasFile =
    "k:/test/mdf/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";

constexpr std::string_view kLargeTime = "t";
constexpr std::string_view kLargeFrameCounter = "FrameCounter_VC0";
constexpr std::string_view kLargeFrameData = "VideoRawdata_VC0";
constexpr std::string_view kBenchMarkFile = "K:/test/mdf/net/test.mf4";
constexpr std::string_view kCssTestFile = "css_test";
constexpr std::string_view kCanFdFile = "test_canfd";

using MdfList = std::map<std::string, std::string, util::string::IgnoreCase>;
MdfList kMdfList;

std::string GetMdfFile(const std::string &name) {
  auto itr = kMdfList.find(name);
  return itr == kMdfList.cend() ? std::string() : itr->second;
}

double ConvertToMs(uint64_t diff) {
  auto temp = static_cast<double>(diff);
  temp /= 1'000'000;
  return temp;
}

double ConvertToSec(uint64_t diff) {
  auto temp = static_cast<double>(diff);
  temp /= 1'000'000'000;
  return temp;
}

void MdfLogFunction(const MdfLocation& location, MdfLogSeverity severity,
                    const std::string& text) {
  LogStringEx(location.line, location.column, location.file, location.function,
              static_cast<LogSeverity>(severity), text);
}

}  // namespace

namespace mdf::test {

void TestRead::SetUpTestSuite() {
  // Set up the log file to log to console
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.Type(util::log::LogType::LogToConsole);
  log_config.CreateDefaultLogger();

  MdfLogStream::SetLogFunction1(MdfLogFunction);

  kMdfList.clear();
  try {
    for (const auto &entry : recursive_directory_iterator(mdf_source_dir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &fullname = entry.path();
      const auto stem = fullname.stem().string();
      if (kMdfList.find(stem) == kMdfList.cend() && IsMdfFile(fullname.string())) {
        // std::cout << "Found MDF file. File: " << stem << std::endl;
        kMdfList.emplace(stem, fullname.string());
      }
    }
  } catch (const std::exception &error) {
    std::cout << "Failed to fetch the MDF test files. Error: "
                << error.what()  << std::endl;
  }
}

void TestRead::TearDownTestSuite() {
  MdfLogStream::SetLogFunction1(nullptr);
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestRead, Read)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found." );
  }

  for (const auto& [name, filename] : kMdfList) {
    const auto start1 = steady_clock::now();

    MdfReader oRead(filename);
    EXPECT_TRUE(oRead.IsOk()) << name;
    const auto* mdf_file = oRead.GetFile();

    EXPECT_TRUE(mdf_file != nullptr) << name;
    EXPECT_TRUE(oRead.ReadEverythingButData()) << name;

    const auto stop1 = steady_clock::now();
    const auto diff1 = duration_cast<milliseconds>(stop1 - start1);

    auto* dg_group = oRead.GetDataGroup(0);
    if (dg_group == nullptr) {
      continue;
    }
    const auto start2 = steady_clock::now();
    ISampleObserver observer(*dg_group);
    oRead.ReadData(*dg_group);

    const auto stop2 = steady_clock::now();
    const auto diff2 = duration_cast<milliseconds>(stop2 - start2);
    std::cout << "File: " << name << " (" << diff1.count()
              << "/" << diff2.count() << " ms)" << std::endl;
  }
}

TEST_F(TestRead, MdfFile)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found.");
  }
  for (const auto &[name, filename] : kMdfList) {
    MdfReader oRead(filename);
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << name;
    const auto *mdf_file = oRead.GetFile();
    ASSERT_TRUE(mdf_file != nullptr) << name;
    if (mdf_file->IsMdf4()) {
      const auto *file4 = dynamic_cast<const Mdf4File *>(mdf_file);
      ASSERT_TRUE(file4 != nullptr);
      const auto &hd4 = file4->Hd();
      const auto &dg_list = hd4.Dg4();
      LOG_INFO() << " File: " << name
                 << ", Nof Measurement: " << dg_list.size();
      EXPECT_FALSE(dg_list.empty()) << name;
    } else {
      const auto *file3 = dynamic_cast<const Mdf3File *>(mdf_file);
      EXPECT_TRUE(file3 != nullptr);
    }
  }
}

TEST_F(TestRead, IdBlock)  // NOLINT
{
  const std::string file = GetMdfFile("Vector_CustomExtensions_CNcomment");
  // Check that the file exists. If not, skip the test
  if (file.empty()) {
    GTEST_SKIP_("Comment file doesn't exist.");
  }
  MdfReader oRead(file);
  EXPECT_TRUE(oRead.IsOk()) << file;

  const auto *f = dynamic_cast<const Mdf4File *>(oRead.GetFile());
  EXPECT_TRUE(f != nullptr) << file;

  const auto &id = f->Id();
  EXPECT_STREQ(id.FileId().c_str(), "MDF");
  EXPECT_STREQ(id.VersionString().c_str(), "4.10");
  EXPECT_STREQ(id.ProgramId().c_str(), "MCD11.01");
  EXPECT_EQ(id.Version(), 410);
}

TEST_F(TestRead, HeaderBlock)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP();
  }
  int count = 0;
  for (const auto &[name, filename] : kMdfList) {
    MdfReader oRead(filename);
    EXPECT_TRUE(oRead.ReadEverythingButData()) << name;

    const auto *mdf_file = oRead.GetFile();
    if (!mdf_file->IsMdf4()) {
      continue;
    }
    const auto *mdf4_file = dynamic_cast<const Mdf4File *>(mdf_file);
    const auto &hd4 = mdf4_file->Hd();
    EXPECT_EQ(&hd4, hd4.HeaderBlock());

    for (const auto& dg4 : hd4.Dg4()) {
      EXPECT_EQ(&hd4, dg4->HeaderBlock());
      for (const auto& cg4 : dg4->Cg4()) {
        EXPECT_EQ(&hd4, cg4->HeaderBlock());
        for (const auto& cn4 : cg4->Cn4()) {
          EXPECT_EQ(&hd4, cn4->HeaderBlock());
        }
      }
    }
    ++count;
    // Only need to check 5 files
    if (count > 5) {
      break;
    }
  }
}

TEST_F(TestRead, Benchmark) {
  try {
    if (!exists(kBenchMarkFile)) {
      GTEST_SKIP();
    }
  } catch (const std::exception& ) {
    GTEST_SKIP();
  }

  {
    MdfReader oRead(kBenchMarkFile);
    const auto start = steady_clock::now();
    oRead.ReadMeasurementInfo();
    const auto stop = steady_clock::now();
    std::chrono::duration<double> diff = duration_cast<milliseconds>(stop - start);
    std::cout << "Read Measurement Info TrueNas: " << diff.count()
              << " ms" << std::endl;
  }
  {
    const auto start = steady_clock::now();
    MdfReader oRead(kBenchMarkFile);
    oRead.ReadEverythingButData();
    const auto *file = oRead.GetFile();
    DataGroupList dg_list;
    file->DataGroups(dg_list);
    for (auto *dg : dg_list) {
      ChannelObserverList observer_list;
      auto cg_list = dg->ChannelGroups();
      for (const auto *cg : cg_list) {
        CreateChannelObserverForChannelGroup(*dg, *cg, observer_list);
      }
      oRead.ReadData(*dg);

      double eng_value = 0;
      bool valid = true;
      for (const auto &channel : observer_list) {
        size_t samples = channel->NofSamples();
        for (size_t sample = 0; sample < samples; ++sample) {
          valid = channel->GetEngValue(sample, eng_value);
        }
      }

    }

    const auto stop = steady_clock::now();
    std::chrono::duration<double> diff = duration_cast<milliseconds>(stop - start);
    std::cout << "Everything + Conversion (TrueNas): " << diff.count()
              << " ms" << std::endl;
  }
}

TEST_F(TestRead, TestLargeFile) {

  std::array<std::string_view, 3> file_list = {
      kLargeLocalSsdFile,
      kLargeLocalDriveFile,
      kLargeNasFile};
  std::array<std::string_view, 3> storage_list = {
      "Local SSD Drive",
      "Local Mechanical Drive",
      "NAS"
  };

  for (size_t file = 0; file < file_list.size(); ++file) {
    const std::string test_file(file_list[file]);
    const std::string storage_type(storage_list[file]);

    try {
      const auto full_path = path(test_file);
      if (!exists(full_path)) {
        throw std::runtime_error("File doesn't exist");
      }
    } catch (const std::exception&) {
      continue;
    }

    {
      std::cout << "BENCHMARK Large (" << storage_type << ") Storage ReadData(without VLSD data)" << std::endl;

      const auto start_open = MdfHelper::NowNs();
      MdfReader oRead(test_file);
      const auto stop_open = MdfHelper::NowNs();
      std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

      const auto start_info = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadEverythingButData()) << oRead.ShortName();
      const auto stop_info = MdfHelper::NowNs();
      std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

      const auto *header = oRead.GetHeader();
      ASSERT_TRUE(header != nullptr);

      auto *last_dg = header->LastDataGroup();
      ASSERT_TRUE(last_dg != nullptr);

      auto channel_groups = last_dg->ChannelGroups();
      ASSERT_FALSE(channel_groups.empty());

      auto *channel_group = channel_groups[0];
      ASSERT_TRUE(channel_group != nullptr);

      auto *time_channel = channel_group->GetChannel(kLargeTime);
      ASSERT_TRUE(time_channel != nullptr);

      auto *counter_channel = channel_group->GetChannel(kLargeFrameCounter);
      ASSERT_TRUE(counter_channel != nullptr);

      auto time_observer = CreateChannelObserver(*last_dg,
                                                 *channel_group, *time_channel);
      auto counter_observer = CreateChannelObserver(*last_dg,
                                                    *channel_group, *counter_channel);

      const auto start_data = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadData(*last_dg)) << oRead.ShortName();
      const auto stop_data = MdfHelper::NowNs();
      std::cout << "Read Data (s): " << ConvertToSec(stop_data - start_data) << std::endl;

      last_dg->ClearData();
      time_observer.reset();
      counter_observer.reset();
      std::cout << std::endl;
    }

    {
      std::cout << "BENCHMARK Large (" << storage_type << ") Storage ReadData(with VLSD data)" << std::endl;

      const auto start_open = MdfHelper::NowNs();
      MdfReader oRead(test_file);
      const auto stop_open = MdfHelper::NowNs();
      std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

      const auto start_info = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadEverythingButData()) << oRead.ShortName();
      const auto stop_info = MdfHelper::NowNs();
      std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

      const auto *header = oRead.GetHeader();
      ASSERT_TRUE(header != nullptr);

      auto *last_dg = header->LastDataGroup();
      ASSERT_TRUE(last_dg != nullptr);

      auto channel_groups = last_dg->ChannelGroups();
      ASSERT_FALSE(channel_groups.empty());

      auto *channel_group = channel_groups[0];
      ASSERT_TRUE(channel_group != nullptr);

      auto *time_channel = channel_group->GetChannel(kLargeTime);
      ASSERT_TRUE(time_channel != nullptr);

      auto *counter_channel = channel_group->GetChannel(kLargeFrameCounter);
      ASSERT_TRUE(counter_channel != nullptr);
      auto *frame_channel = channel_group->GetChannel(kLargeFrameData);
      ASSERT_TRUE(frame_channel != nullptr);

      auto time_observer = CreateChannelObserver(*last_dg,
                                                 *channel_group, *time_channel);
      auto counter_observer = CreateChannelObserver(*last_dg,
                                                    *channel_group, *counter_channel);
      auto frame_observer = CreateChannelObserver(*last_dg,
                                                  *channel_group, *frame_channel);
      frame_observer->ReadVlsdData(false);

      const auto start_data = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadData(*last_dg)) << oRead.ShortName();
      auto offset_list = frame_observer->GetOffsetList();
      EXPECT_FALSE(offset_list.empty());
      const auto stop_data = MdfHelper::NowNs();
      std::cout << "Read Offset Data (s): " << ConvertToSec(stop_data - start_data) << std::endl;

//      size_t sample_count = 0;
      std::function callback = [&](uint64_t offset, const std::vector<uint8_t> &buffer) {
        const bool offset_exist = std::any_of(offset_list.cbegin(),offset_list.cend(),
                                              [&] (uint64_t off) {
          return off == offset;
        });
        EXPECT_TRUE(offset_exist) << "Offset: " << offset << std::endl;

//        std::cout << "Offset: " << offset << "/" << offset_list[sample_count]
//                  << ", Size: " << buffer.size() << std::endl;
//        ++sample_count;
      };
      const auto start_vlsd = MdfHelper::NowNs();
      while (offset_list.size() > 100) {
        offset_list.pop_back();
      }
      EXPECT_TRUE(oRead.ReadVlsdData(*last_dg, *frame_channel, offset_list, callback)) << oRead.ShortName();
      const auto stop_vlsd = MdfHelper::NowNs();

      std::cout << "Read 100 VLSD Data (s): " << ConvertToSec(stop_vlsd - start_vlsd) << std::endl;


      last_dg->ClearData();
      time_observer.reset();
      counter_observer.reset();

      std::cout << std::endl;
    }
  }
}

TEST_F(TestRead, TestExampleCrash) {
    const std::string crash_file = GetMdfFile("crashfile");
    if (crash_file.empty()) {
      GTEST_SKIP();
    }

    MdfReader reader(crash_file);  // Note the file is now open.

    // Read all blocks but not the raw data and attachments.
    // This reads in the block information into memory.
    EXPECT_TRUE(reader.ReadEverythingButData());

    const auto* mdf_file = reader.GetFile(); // Get the file interface.
    DataGroupList dg_list;                   // Get all measurements.
    mdf_file->DataGroups(dg_list);
    EXPECT_EQ(dg_list.size(), 2);

    // In this example, we read in all sample data and fetch all values.
    for (auto* dg4 : dg_list) {
      // Subscribers hold the sample data for a channel.
      // You should normally only subscribe on some channels.
      // We need a list to hold them.
      ChannelObserverList subscriber_list;
      const auto cg_list = dg4->ChannelGroups();
      for (const auto* cg4 : cg_list ) {
        const auto cn_list = cg4->Channels();
        for (const auto* cn4 : cn_list) {
          // Create a subscriber and add it to the temporary list
          auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
          EXPECT_TRUE(sub);
          subscriber_list.push_back(std::move(sub));
        }
      }

      // Now it is time to read in all samples
      EXPECT_TRUE(reader.ReadData(*dg4)); // Read raw data from the file.
      std::string channel_value; // Channel value (no scaling)
      std::string eng_value; // Engineering value
      for (auto& obs : subscriber_list) {
        EXPECT_GT(obs->NofSamples(), 0);
        std::cout << obs->Name() << " Samples: " << obs->NofSamples() << std::endl;
        for (size_t sample = 0; sample < obs->NofSamples(); ++sample) {
          const auto channel_valid = obs->GetChannelValue(sample, channel_value);
          const auto eng_valid = obs->GetEngValue(sample, eng_value);
          // You should do something with data here
          std::cout << sample << ": " << (eng_valid ? eng_value : "*") << std::endl;
        }
        std::cout << std::endl;
      }

      dg4->ClearData();
    }
    reader.Close(); // Close the file

}

TEST_F(TestRead, TestPartialRead) {
  try {
    const auto full_path = path(kLargeNasFile);
    if (!exists(full_path)) {
      throw std::runtime_error("File doesn't exist");
    }
  } catch (const std::exception &) {
    GTEST_SKIP();
  }

  std::cout << "Start reading large 22GB file (NAS storage." << std::endl;

  const auto start_open = MdfHelper::NowNs();
  MdfReader oRead(kLargeNasFile);
  const auto stop_open = MdfHelper::NowNs();
  std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open)
            << std::endl;

  const auto start_info = MdfHelper::NowNs();
  EXPECT_TRUE(oRead.ReadEverythingButData()) << oRead.ShortName();
  const auto stop_info = MdfHelper::NowNs();
  std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info)
            << std::endl;

  const auto *header = oRead.GetHeader();
  auto *last_dg = header->LastDataGroup();
  auto channel_groups = last_dg->ChannelGroups();
  auto *channel_group = channel_groups[0];
  auto *time_channel = channel_group->GetChannel(kLargeTime);
  auto *counter_channel = channel_group->GetChannel(kLargeFrameCounter);
  auto *data_channel = channel_group->GetChannel(kLargeFrameData);

  auto time_observer =
      CreateChannelObserver(*last_dg, *channel_group, *time_channel);
  auto counter_observer =
      CreateChannelObserver(*last_dg, *channel_group, *counter_channel);
  auto data_observer =
      CreateChannelObserver(*last_dg, *channel_group, *data_channel);

  const auto start_data = MdfHelper::NowNs();
  EXPECT_TRUE(oRead.ReadPartialData(*last_dg, 0, 1)) << oRead.ShortName();
  const auto stop_data = MdfHelper::NowNs();
  std::cout << "Read 1 sample data (ms): "
            << ConvertToMs(stop_data - start_data) << std::endl;

  for (size_t sample = 0; sample < time_observer->NofSamples(); ++sample) {
    double time = 0;
    bool valid = time_observer->GetEngValue(sample, time);
    if (sample == 0) {
      EXPECT_TRUE(valid);
    } else {
      EXPECT_FALSE(valid);
    }
    std::vector<uint8_t> data;
    valid = data_observer->GetEngValue(sample, data);
    if (sample == 0) {
      EXPECT_GT(data.size(), 0);
      EXPECT_TRUE(valid);
    } else {
      EXPECT_FALSE(valid);
      EXPECT_TRUE(data.empty());
    }
  }

  last_dg->ClearData();
  time_observer.reset();
  counter_observer.reset();
  data_observer.reset();
}

TEST_F(TestRead, TestNotFinalized) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found.");
  }

  // Find all files that are not finalized.
  MdfList test_list;
  for (const auto& [name, filename] : kMdfList ) {
    MdfReader reader(filename);
    if (!reader.IsFinalized()) {
      test_list.emplace(name, filename);
    }
  }
  EXPECT_FALSE(test_list.empty());
  for (const auto& [name1, filename1] : test_list) {
    MdfReader reader(filename1);
    EXPECT_TRUE(reader.ReadEverythingButData());
    EXPECT_TRUE(reader.IsFinalized()) << name1;
    std::cout << "Finalized File: " << name1 << std::endl;

  }

}

TEST_F(TestRead, TruncatedNotFinalized) {
  const path mdf_file = temp_directory_path() / "truncated_not_finalized.mf4";
  std::error_code err;
  std::filesystem::remove(mdf_file, err);
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer && writer->Init(mdf_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel();
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    auto* channel = group->CreateChannel();
    channel->Name("Value");
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(8);
    writer->CompressData(false);
    writer->PreTrigTime(0);
    ASSERT_TRUE(writer->InitMeasurement());
    uint64_t time = MdfHelper::NowNs();
    writer->StartMeasurement(time);
    for (size_t sample = 0; sample < 100; ++sample) {
      channel->SetChannelValue(static_cast<double>(sample));
      writer->SaveSample(*group, time);
      time += 1'000'000;
    }
    writer->StopMeasurement(time);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }
  {
    // Make it look like a logger file that was not finalized. The CG block has
    // no cycle count, and the CG and DT block are flagged for an update.
    std::fstream file(mdf_file, std::ios::in | std::ios::out | std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(file)), {});
    const auto cg = bytes.find("##CG");
    ASSERT_NE(cg, std::string::npos);
    uint64_t links = 0;
    std::memcpy(&links, bytes.data() + cg + 16, sizeof(links));
    const uint64_t no_samples = 0;
    file.seekp(static_cast<std::streamoff>(cg + 24 + 8 * links + 8));
    file.write(reinterpret_cast<const char*>(&no_samples), sizeof(no_samples));
    file.seekp(0);
    file.write("UnFinMF ", 8);
    file.seekp(60);
    file.write("\x05\x00", 2);
  }
  // Cut the last 16 byte record in half.
  resize_file(mdf_file, file_size(mdf_file) - 8);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.ReadEverythingButData());
  EXPECT_TRUE(reader.IsFinalized());
  const auto* group = reader.GetDataGroup(0)->ChannelGroups()[0];
  EXPECT_EQ(group->NofSamples(), 100);  // The cut record counts in full.
}

TEST_F(TestRead, MdfIdentifierWithUnfinalizedFlags) {
  const path mdf_file = temp_directory_path() / "mdf_unfinalized_flags.mf4";
  std::error_code err;
  std::filesystem::remove(mdf_file, err);
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer && writer->Init(mdf_file.string()));
    writer->GetFile()->MinorVersion(10);  // The flags count in any version.
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel();
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    writer->CompressData(false);
    ASSERT_TRUE(writer->InitMeasurement());
    uint64_t time = MdfHelper::NowNs();
    writer->StartMeasurement(time);
    for (size_t sample = 0; sample < 10; ++sample) {
      writer->SaveSample(*group, time);
      time += 1'000'000;
    }
    writer->StopMeasurement(time);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }
  const auto nof_samples = [&mdf_file] {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.ReadEverythingButData());
    return reader.GetDataGroup(0)->ChannelGroups()[0]->NofSamples();
  };
  std::fstream file(mdf_file, std::ios::in | std::ios::out | std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(file)), {});
  // The identifier stays "MDF", but the CG and DT block are flagged for an
  // update. Finalizing keeps the right cycle count.
  file.seekp(60);
  file.write("\x05\x00", 2);
  file.flush();
  EXPECT_EQ(nof_samples(), 10);

  // Finalizing restores a missing cycle count.
  const auto cg = bytes.find("##CG");
  ASSERT_NE(cg, std::string::npos);
  uint64_t links = 0;
  std::memcpy(&links, bytes.data() + cg + 16, sizeof(links));
  const uint64_t no_samples = 0;
  file.seekp(static_cast<std::streamoff>(cg + 24 + 8 * links + 8));
  file.write(reinterpret_cast<const char*>(&no_samples), sizeof(no_samples));
  file.flush();
  EXPECT_EQ(nof_samples(), 10);
}

TEST_F(TestRead, TruncatedVlsdLength) {
  const path mdf_file = temp_directory_path() / "truncated_vlsd_length.mf4";
  std::error_code err;
  std::filesystem::remove(mdf_file, err);
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
    ASSERT_TRUE(writer && writer->Init(mdf_file.string()));
    writer->BusType(MdfBusType::CAN);
    writer->StorageType(MdfStorageType::VlsdStorage);
    writer->MaxLength(8);
    writer->MandatoryMembersOnly(true);
    ASSERT_TRUE(writer->CreateBusLogConfiguration());
    writer->PreTrigTime(0);
    writer->CompressData(false);
    auto* frames = writer->Header()->LastDataGroup()->GetChannelGroup("CAN_DataFrame");
    ASSERT_TRUE(frames != nullptr);
    ASSERT_TRUE(writer->InitMeasurement());
    uint64_t time = MdfHelper::NowNs();
    writer->StartMeasurement(time);
    for (size_t sample = 0; sample < 10; ++sample) {
      CanMessage msg;
      msg.MessageId(123);
      msg.DataBytes(std::vector<uint8_t>(8, static_cast<uint8_t>(0xA0 + sample)));
      writer->SaveCanMessage(*frames, time, msg);
      time += 1'000'000;
    }
    writer->StopMeasurement(time);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }
  std::string bytes;
  {
    // Make it look like a logger file that was not finalized. The CG blocks
    // have no cycle count and no VLSD size, and they are flagged for an update.
    std::fstream file(mdf_file, std::ios::in | std::ios::out | std::ios::binary);
    bytes.assign(std::istreambuf_iterator<char>(file), {});
    for (auto cg = bytes.find("##CG"); cg != std::string::npos;
         cg = bytes.find("##CG", cg + 4)) {
      uint64_t links = 0;
      std::memcpy(&links, bytes.data() + cg + 16, sizeof(links));
      const auto data = static_cast<std::streamoff>(cg + 24 + 8 * links);
      const uint64_t zero = 0;
      file.seekp(data + 8);
      file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
      if ((bytes[data + 16] & CgFlag::VlsdChannel) != 0) {
        file.seekp(data + 24);
        file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
      }
    }
    file.seekp(0);
    file.write("UnFinMF ", 8);
    file.seekp(60);
    file.write("\x25\x00", 2);
  }
  // The file ends 2 bytes into the length of the VLSD record of the last frame.
  const auto last = bytes.rfind(std::string("\x08\x00\x00\x00", 4) + std::string(8, '\xA9'));
  ASSERT_NE(last, std::string::npos);
  resize_file(mdf_file, last + 2);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* frames = reader.GetDataGroup(0)->GetChannelGroup("CAN_DataFrame");
  ASSERT_TRUE(frames != nullptr);
  EXPECT_EQ(frames->NofSamples(), 9);
}

TEST_F(TestRead, TruncatedMdf3) {
  const path mdf_file = temp_directory_path() / "truncated_mdf3.mdf";
  std::error_code err;
  std::filesystem::remove(mdf_file, err);
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);
    ASSERT_TRUE(writer && writer->Init(mdf_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel();
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    auto* channel = group->CreateChannel();
    channel->Name("Value");
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(8);
    writer->PreTrigTime(0);
    ASSERT_TRUE(writer->InitMeasurement());
    uint64_t time = MdfHelper::NowNs();
    writer->StartMeasurement(time);
    for (size_t sample = 0; sample < 10; ++sample) {
      channel->SetChannelValue(static_cast<double>(sample + 1));
      writer->SaveSample(*group, time);
      time += 1'000'000;
    }
    writer->StopMeasurement(time);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }
  {
    // The data block ends the file.
    std::ifstream file(mdf_file, std::ios::binary);
    file.seekg(-8, std::ios::end);
    double last = 0;
    file.read(reinterpret_cast<char*>(&last), sizeof(last));
    ASSERT_EQ(last, 10.0);
  }
  // The file ends in the middle of the second last 16 byte record.
  resize_file(mdf_file, file_size(mdf_file) - 24);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.ReadEverythingButData());
  auto* data_group = reader.GetDataGroup(0);
  auto* group = data_group->ChannelGroups()[0];
  auto observer = CreateChannelObserver(*data_group, *group,
                                        *group->GetChannel("Value"));
  reader.ReadData(*data_group);
  double value = 0;
  EXPECT_TRUE(observer->GetEngValue(7, value));
  EXPECT_EQ(value, 8.0);
  EXPECT_FALSE(observer->GetEngValue(8, value));
  EXPECT_FALSE(observer->GetEngValue(9, value));
}

TEST_F(TestRead, TestCssChannelObservers) {
  const std::string& test_file = GetMdfFile(kCssTestFile.data());
  const auto find_test_file = kMdfList.find(kCssTestFile.data());
  if (find_test_file == kMdfList.cend()) {
    GTEST_SKIP_("No CSS test file found.");
  }

  const auto start1 = steady_clock::now();

  MdfReader reader(test_file);
  ASSERT_TRUE(reader.IsOk());
  reader.ReadEverythingButData();
  const auto stop1 = steady_clock::now();
  const auto diff1 = duration_cast<milliseconds>(stop1 - start1);

  const auto* mdf_file = reader.GetFile();
  ASSERT_TRUE(mdf_file != nullptr);
  DataGroupList dg_list;
  mdf_file->DataGroups(dg_list);
  for (IDataGroup* data_group : dg_list) {
    if (data_group == nullptr) {
      continue;
    }
    ChannelObserverList observer_list;
    CreateChannelObserverForDataGroup(*data_group, observer_list);
    reader.ReadData(*data_group);
    data_group->ClearData();
  }
  reader.Close();

  const auto stop2 = steady_clock::now();
  const auto diff2 = duration_cast<milliseconds>(stop2 - stop1);

  std:: cout << "File: " << kCssTestFile << " (" << diff1.count()
            << "/" << diff2.count() << " ms)" << std::endl;

}

TEST_F(TestRead, TestCssSampleObserver) {
  const std::string& test_file = GetMdfFile(kCssTestFile.data());
  const auto find_test_file = kMdfList.find(kCssTestFile.data());
  if (find_test_file == kMdfList.cend()) {
    GTEST_SKIP_("No CSS test file found.");
  }

  const auto start1 = steady_clock::now();

  MdfReader reader(test_file);
  ASSERT_TRUE(reader.IsOk());
  reader.ReadEverythingButData();
  const auto stop1 = steady_clock::now();
  const auto diff1 = duration_cast<milliseconds>(stop1 - start1);

  const auto* mdf_file = reader.GetFile();
  ASSERT_TRUE(mdf_file != nullptr);
  DataGroupList dg_list;
  mdf_file->DataGroups(dg_list);
  for (IDataGroup* data_group : dg_list) {
    if (data_group == nullptr) {
      continue;
    }
    ISampleObserver observer(*data_group);

    reader.ReadData(*data_group);
  }
  reader.Close();

  const auto stop2 = steady_clock::now();
  const auto diff2 = duration_cast<milliseconds>(stop2 - stop1);

  std:: cout << "File: " << kCssTestFile << " (" << diff1.count()
            << "/" << diff2.count() << " ms)" << std::endl;

}


TEST_F(TestRead, TestStreamInterface)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found.");
  }
  // Read into memory and then parse the memory.

  for (const auto &[name, filename] : kMdfList) {
    uint64_t file_size = 0;
    // Read in file into memory

    ASSERT_TRUE(IsMdfFile(filename));

    try {
      const path fullname(filename);
      file_size = std::filesystem::file_size(fullname);
     } catch (std::exception& err) {
      ADD_FAILURE() << err.what();
      continue;
    }

    if (file_size > 1'000'000'000) {
      // Skip large files.
      continue;
    }
    std::vector<char> file_array;
    file_array.reserve(file_size);

    back_insert_device<std::vector<char>> array_device(file_array);
    stream_buffer< back_insert_device<std::vector<char> >> out_buffer(array_device);
    std::ostream out_array(&out_buffer);

    std::ifstream in_file(filename, std::ios_base::in | std::ios_base::binary);
    for ( int input = in_file.get(); !in_file.eof(); input = in_file.get()  ) {
      if (input>= 0) {
        out_array.put(static_cast<char>(input));
      }
    }
    in_file.close();
    out_buffer.pubsync(); // Doing a flush of buffer to vector
    out_buffer.close();

    if (file_array.empty()) {
      ADD_FAILURE() << "File is empty. File: " << name;
      continue;
    }

    std::shared_ptr<std::streambuf> input_buffer =
        std::make_shared< stream_buffer<array_source> >(file_array.data(), file_array.size());

    ASSERT_TRUE(IsMdfFile(*input_buffer));

    MdfReader oRead(input_buffer);
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << name;
    const auto *mdf_file = oRead.GetFile();
    ASSERT_TRUE(mdf_file != nullptr) << name;
    if (mdf_file->IsMdf4()) {
      const auto *file4 = dynamic_cast<const Mdf4File *>(mdf_file);
      ASSERT_TRUE(file4 != nullptr) << name;
      const auto &hd4 = file4->Hd();
      const auto &dg_list = hd4.Dg4();
      std::cout <<"File: " << name << " (" << file_size / 1'000
                << " kB) , Nof Measurement: " << dg_list.size() << std::endl;
      EXPECT_FALSE(dg_list.empty()) << name;
    } else {
      const auto *file3 = dynamic_cast<const Mdf3File *>(mdf_file);
      EXPECT_TRUE(file3 != nullptr);
      ASSERT_TRUE(file3 != nullptr) << name;
      const auto &hd3 = file3->Hd();
      const auto &dg_list = hd3.Dg3();
      std::cout <<"File: " << name << " (" << file_size / 1'000
                << " kB) , Nof Measurement: " << dg_list.size() << std::endl;
      EXPECT_FALSE(dg_list.empty()) << name;
    }
  }
}

TEST_F(TestRead, TestCanFd) {
  std::string test_file = GetMdfFile(kCanFdFile.data());
  if (test_file.empty()) {
    GTEST_SKIP_("CAN FD file not found");
  }

  MdfReader reader(test_file);
  const bool read_config = reader.ReadEverythingButData();
  ASSERT_TRUE(read_config);

  const MdfFile* mf4_file = reader.GetFile();
  ASSERT_TRUE(mf4_file != nullptr);

  std::vector<IDataGroup*> dg_list;
  mf4_file->DataGroups(dg_list);
  EXPECT_EQ(dg_list.size(), 3);

  for (IDataGroup* data_group : dg_list ) {
    ASSERT_TRUE(data_group != nullptr);

    // Subscribe on all channels
    ChannelObserverList observer_list;
    CreateChannelObserverForDataGroup(*data_group, observer_list);
    EXPECT_FALSE(observer_list.empty());

    const bool read = reader.ReadData(*data_group);
    EXPECT_TRUE(read);

    for (uint64_t sample = 0; sample < 5; ++sample) {

      bool valid = false;
      uint16_t channel = 0;
      std::string direction;
      uint32_t can_id = 0;
      std::vector<uint8_t> data_bytes;

      for (const auto &observer : observer_list) {
        if (observer->Name() == "CAN_DataFrame.BusChannel") {
          valid = observer->GetEngValue(sample, channel);
          EXPECT_TRUE(valid);
        } else if (observer->Name() == "CAN_DataFrame.Dir") {
          valid = observer->GetEngValue(sample, direction);
          EXPECT_TRUE(valid);
        } else if (observer->Name() == "CAN_DataFrame.ID") {
          valid = observer->GetEngValue(sample, can_id);
          EXPECT_TRUE(valid);
        } else if (observer->Name() == "CAN_DataFrame.DataBytes") {
          valid = observer->GetEngValue(sample, data_bytes);
          EXPECT_TRUE(valid);
        }
      }
      if (valid) {
        std::cout << "Sample: " << sample
                  << " Channel: " << channel
                  << " Direction: " << direction
                  << " CAN ID: " << can_id
                  << " Data: ";
        std::ostringstream hex; // Needs temporary stream as I change format.
        for (uint8_t data : data_bytes) {
          hex << std::hex << std::uppercase << std::setw(2)
                    << std::setfill('0') << static_cast<int>(data) << " ";
        }
        std::cout << hex.str() << std::endl;

      }
    }

  }
}

TEST_F(TestRead, SignedVirtualMaster) {
  // imc FAMOS writes the virtual master as a signed integer with 0 bits.
  const path mdf_file = temp_directory_path() / "signed_virtual_master.mf4";
  std::error_code err;
  std::filesystem::remove(mdf_file, err);
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer && writer->Init(mdf_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel();
    master->Name("Time");
    master->Type(ChannelType::VirtualMaster);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::SignedIntegerLe);
    auto* conversion = master->CreateChannelConversion();
    conversion->Type(ConversionType::Linear);
    conversion->Parameter(0, 0.0);
    conversion->Parameter(1, 0.001);
    auto* channel = group->CreateChannel();
    channel->Name("Value");
    channel->DataType(ChannelDataType::UnsignedIntegerLe);
    channel->DataBytes(4);
    writer->CompressData(false);
    writer->PreTrigTime(0);
    ASSERT_TRUE(writer->InitMeasurement());
    uint64_t time = MdfHelper::NowNs();
    writer->StartMeasurement(time);
    for (size_t sample = 0; sample < 300; ++sample) {
      channel->SetChannelValue(sample);
      writer->SaveSample(*group, time);
      time += 1'000'000;
    }
    writer->StopMeasurement(time);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.ReadEverythingButData());
  auto* data_group = reader.GetDataGroup(0);
  const auto* group = data_group->ChannelGroups()[0];
  const auto* master = group->GetChannel("Time");
  ASSERT_TRUE(master != nullptr);
  EXPECT_EQ(master->DataType(), ChannelDataType::SignedIntegerLe);
  EXPECT_EQ(master->BitCount(), 0U);
  auto observer = CreateChannelObserver(*data_group, *group, *master);
  ASSERT_TRUE(reader.ReadData(*data_group));
  ASSERT_EQ(observer->NofSamples(), 300);
  int64_t value = 0;
  EXPECT_TRUE(observer->GetChannelValue(299, value));
  EXPECT_EQ(value, 299);
  double time = 0.0;
  EXPECT_TRUE(observer->GetEngValue(299, time));
  EXPECT_DOUBLE_EQ(time, 0.299);
}

TEST_F(TestRead, OnlyVirtualChannels) {
  // A group with only virtual channels has records without bytes.
  const path mdf_file = temp_directory_path() / "only_virtual_channels.mf4";
  std::error_code err;
  std::filesystem::remove(mdf_file, err);
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer && writer->Init(mdf_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel();
    master->Name("Time");
    master->Type(ChannelType::VirtualMaster);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::UnsignedIntegerLe);
    auto* conversion = master->CreateChannelConversion();
    conversion->Type(ConversionType::Linear);
    conversion->Parameter(0, 0.0);
    conversion->Parameter(1, 0.5);
    auto* index = group->CreateChannel();
    index->Name("Index");
    index->Type(ChannelType::VirtualData);
    index->DataType(ChannelDataType::UnsignedIntegerLe);
    writer->CompressData(false);
    writer->PreTrigTime(0);
    ASSERT_TRUE(writer->InitMeasurement());
    uint64_t time = MdfHelper::NowNs();
    writer->StartMeasurement(time);
    for (size_t sample = 0; sample < 4; ++sample) {
      writer->SaveSample(*group, time);
      time += 500'000'000;
    }
    writer->StopMeasurement(time);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.ReadEverythingButData());
  auto* data_group = reader.GetDataGroup(0);
  const auto* group = data_group->ChannelGroups()[0];
  ASSERT_EQ(group->NofSamples(), 4);
  auto time = CreateChannelObserver(*data_group, *group,
                                    *group->GetChannel("Time"));
  auto index = CreateChannelObserver(*data_group, *group,
                                     *group->GetChannel("Index"));
  ASSERT_TRUE(reader.ReadData(*data_group));
  double value = 0.0;
  EXPECT_TRUE(time->GetEngValue(3, value));
  EXPECT_DOUBLE_EQ(value, 1.5);
  uint64_t sample = 0;
  EXPECT_TRUE(index->GetChannelValue(3, sample));
  EXPECT_EQ(sample, 3);
}

TEST_F(TestRead, TestReadData) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files.");
  }
  size_t read_count = 0;
  for (const auto& [name, test_file]: kMdfList) {
    MdfReader reader(test_file);
    const bool read_config = reader.ReadEverythingButData();
    ASSERT_TRUE(read_config) << name;

    const MdfFile* mf4_file = reader.GetFile();
    ASSERT_TRUE(mf4_file != nullptr);

    std::vector<IDataGroup*> dg_list;
    mf4_file->DataGroups(dg_list);
    if (dg_list.empty()) {
      LOG_TRACE() << "No data groups in the test file. File: " << test_file;
      continue;
    }
    const auto test_file_size = file_size(test_file);
    if (test_file_size > 10'000'000) {
     // LOG_TRACE() << "Skipped file due to size. File: " << name
     //     << ", Size: " << static_cast<double>(test_file_size)/ 1E6 << "MB";
      continue;
    }
    for (IDataGroup* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);
      ChannelObserverList observers;
      CreateChannelObserverForDataGroup(*data_group, observers);
      EXPECT_TRUE(reader.ReadData(*data_group)) << test_file;
    }
    ++read_count;
  }
  std::cout << "Tested " << read_count << " files." << std::endl;
}

TEST_F(TestRead, TestPartial) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files.");
  }
  size_t read_count = 0;
  for (const auto& [name, test_file]: kMdfList) {
//    if (name != "00000022_Extracted") {
//      continue;
//    }
    MdfReader reader(test_file);
    const bool read_config = reader.ReadEverythingButData();
    ASSERT_TRUE(read_config) << name;

    const MdfFile* mf4_file = reader.GetFile();
    ASSERT_TRUE(mf4_file != nullptr);

    std::vector<IDataGroup*> dg_list;
    mf4_file->DataGroups(dg_list);
    for (IDataGroup* data_group : dg_list) {
      if (data_group == nullptr) {
        continue;
      }

      for (auto* channel_group : data_group->ChannelGroups()) {
        if (channel_group == nullptr) {
          continue;
        }
        ChannelObserverList observers;
        for (auto* channel : channel_group->Channels()) {
          if (channel == nullptr || channel->Type() != ChannelType::Master
            || channel->Sync() != ChannelSyncType::Time) {
            continue;
          }
          observers.emplace_back(CreateChannelObserver(*data_group, *channel_group, *channel));
        }
        if (observers.empty() || channel_group->NofSamples() == 0) {
          continue;
        }
        // Read out firat sample and last sample
        const uint64_t nof_samples = channel_group->NofSamples();
        // reader.ReadData(*data_group);
        reader.ReadPartialData(*data_group, 0, 1);
        reader.ReadPartialData(*data_group, nof_samples - 1, nof_samples);
        for (auto& observer : observers) {
          EXPECT_EQ(observer->NofSamples(), nof_samples) << name;
          double first = 0.0;
          double last = 0.0;
          bool valid = observer->GetEngValue(0,first);
          EXPECT_TRUE(valid) << "First: " << name;
          if (first > 1E10) {
            continue; // Most likely an invalid first time
          }
          valid = observer->GetEngValue(nof_samples -1,last);
          EXPECT_TRUE(valid) << "Last: " << name;
          EXPECT_GE(last, first) << "CG: " << channel_group->Name()
            << ", File: " << test_file;
          //std::cout << "File: " << name << ", Channel: " << observer->Name()
          //  << ", First [s] = " << first
          //  << ", Last [s] = " << last
          //<< std::endl;
        }
      }
    }

    ++read_count;
  }
  std::cout << "Tested " << read_count << " files." << std::endl;
}

}  // namespace mdf::test
