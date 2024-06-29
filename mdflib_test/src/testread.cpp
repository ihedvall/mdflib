/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "testread.h"

#include <chrono>
#include <filesystem>
#include <map>
#include <string>

#include "mdf/mdfreader.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "cn4block.h"

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/stringutil.h"
using namespace std::filesystem;
using namespace util::string;
using namespace mdf;
using namespace mdf::detail;
using namespace util::log;
using namespace std::chrono_literals;

namespace {
const std::string mdf_source_dir =
    "k:/test/mdf";  // Where all source MDF files exist
const std::string log_root_dir = "o:/test";
const std::string log_file = "mdfread.log";

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

using MdfList = std::map<std::string, std::string, util::string::IgnoreCase>;
MdfList mdf_list;

std::string GetMdfFile(const std::string &name) {
  auto itr = mdf_list.find(name);
  return itr == mdf_list.cend() ? std::string() : itr->second;
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
}  // namespace

namespace mdf::test {

void TestRead::SetUpTestSuite() {
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.RootDir(log_root_dir);
  log_config.BaseName(log_file);
  log_config.Type(util::log::LogType::LogToFile);
  log_config.CreateDefaultLogger();

  mdf_list.clear();
  try {
    for (const auto &entry : recursive_directory_iterator(mdf_source_dir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &p = entry.path();

      if (IsMdfFile(p.string())) {
        LOG_INFO() << "Found MDF file. File: " << p.stem().string();
        mdf_list.emplace(p.stem().string(), p.string());
      }
    }
  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to fetch the MDF test files. Error: "
                << error.what();
  }
}

void TestRead::TearDownTestSuite() {
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestRead, MdfReader)  // NOLINT
{
  for (const auto &itr : mdf_list) {
    MdfReader oRead(itr.second);
    EXPECT_TRUE(oRead.IsOk()) << itr.second;
    EXPECT_TRUE(oRead.GetFile() != nullptr) << itr.second;
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << itr.second;
  }
}

TEST_F(TestRead, MdfFile)  // NOLINT
{
  if (mdf_list.empty()) {
    GTEST_SKIP();
  }
  for (const auto &itr : mdf_list) {
    MdfReader oRead(itr.second);
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << itr.second;
    const auto *f = oRead.GetFile();
    EXPECT_TRUE(f != nullptr) << itr.second;
    if (f->IsMdf4()) {
      const auto *f4 = dynamic_cast<const Mdf4File *>(f);
      EXPECT_TRUE(f4 != nullptr);
      const auto &hd = f4->Hd();
      const auto &dg_list = hd.Dg4();
      LOG_INFO() << " File: " << itr.first
                 << ", Nof Measurement: " << dg_list.size();
      EXPECT_FALSE(dg_list.empty()) << itr.second;
    } else {
      const auto *f3 = dynamic_cast<const Mdf3File *>(f);
      EXPECT_TRUE(f3 != nullptr);
    }
  }
}

TEST_F(TestRead, IdBlock)  // NOLINT
{
  const std::string file = GetMdfFile("Vector_CustomExtensions_CNcomment");
  // Check that the file exist. If not skip the test
  if (file.empty()) {
    GTEST_SKIP();
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
  if (mdf_list.empty()) {
    GTEST_SKIP();
  }
  int count = 0;
  for (const auto &itr : mdf_list) {
    MdfReader oRead(itr.second);
    EXPECT_TRUE(oRead.ReadEverythingButData()) << itr.second;

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
    MdfReader oRead(kBenchMarkFile.data());
    const auto start = std::chrono::steady_clock::now();
    oRead.ReadMeasurementInfo();
    const auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = stop - start;
    std::cout << "Read Measurement Info TrueNas: " << diff.count() * 1000
              << " ms" << std::endl;
  }
  {
    const auto start = std::chrono::steady_clock::now();
    MdfReader oRead(kBenchMarkFile.data());
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

    const auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = stop - start;
    std::cout << "Everything + Conversion (TrueNas): " << diff.count() * 1000
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
      MdfReader oRead(test_file.data());
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
      // Subscribers holds the sample data for a channel.
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
      EXPECT_TRUE(reader.ReadData(*dg4)); // Read raw data from file
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

      // Not needed in this example as we delete the subscribers,
      // but it is good practise to remove samples data from memory
      // when it is no longer needed.
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
  MdfReader oRead(kLargeNasFile.data());
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

}  // namespace mdf::test
