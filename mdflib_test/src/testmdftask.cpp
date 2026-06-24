/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "testmdftask.h"

#include <string>
#include <string_view>
#include <map>
#include <filesystem>



#include "mdf/mdftask.h"
#include "mdf/mdflogstream.h"
#include "validatingtask.h"
#include "sortingtask.h"

using namespace std::filesystem;
using MdfList = std::map<std::string, std::string>;

namespace {
constexpr std::string_view kMdfDir = "k:/test/mdf";  // Where all source MDF files exist
constexpr std::string_view kDemoFile = "dSPACE_RealTypes.mf4";
MdfList kMdfList; // List of all MDF files

std::string kTestDir; ///< Temp Dir/test/mdf/task";
bool kSkipTest = false;

std::string GetMdfFile(const std::string& name) {
  auto itr = kMdfList.find(name);
  return itr == kMdfList.cend() ? std::string() : itr->second;
}

}

namespace mdf::test {

TestSampleRecordObserver::TestSampleRecordObserver(
    const IDataGroup& data_group, const IChannelGroup& channel_group,
    uint64_t base_time)
: SampleRecordObserver(data_group, channel_group,base_time) {

}

void TestSampleRecordObserver::OnSampleRecord() {
  SampleRecord record;
  GetSampleRecord(record);
  ++nof_samples_;
}

void TestMdfTask::SetUpTestSuite() {

  try {
    MdfLogStream::SetLogFunction1(MdfLogStream::LogToConsole);

    path temp_dir = temp_directory_path();
    temp_dir.append("test");
    temp_dir.append("mdf");
    temp_dir.append("task");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      MDF_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

    MDF_TRACE() << "Created the test directory. Dir: " << kTestDir;

    kMdfList.clear();

    for (const auto &entry : recursive_directory_iterator(kMdfDir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &fullname = entry.path();
      if (const auto stem = fullname.stem().string();
        kMdfList.find(stem) == kMdfList.cend() && IsMdfFile(fullname.string())) {
        kMdfList.emplace(stem, fullname.string());
      }
    }

    MDF_INFO() << "Found " << kMdfList.size() << " MDF files";
    kSkipTest = kMdfList.empty();

  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestMdfTask::TearDownTestSuite() {
  MDF_TRACE() << "Tear down the test suite.";
  MdfLogStream::ResetLogFunction();
}

TEST_F(TestMdfTask, TestProperties) {

  ValidatingTask task;

  path source_file(kMdfDir);
  source_file.append(kDemoFile);
  const std::string source = source_file.string();
  task.SourceFile(source);
  EXPECT_EQ(task.SourceFile(), source);

  path dest_file(kTestDir);
  dest_file.append(kDemoFile);
  const std::string destination = dest_file.string();
  task.DestinationFile(destination);
  EXPECT_EQ(task.DestinationFile(), destination);

  path temp_file(temp_directory_path());
  temp_file.append(kDemoFile);
  const std::string temporary = temp_file.string();
  task.TempFile(temporary);
  EXPECT_EQ(task.TempFile(), temporary);

  for (auto storage = static_cast<size_t>(MdfStorageType::FixedLengthStorage);
       storage <= static_cast<size_t>(MdfStorageType::MlsdStorage); ++storage) {
    auto type = static_cast<MdfStorageType>(storage);
    task.StorageType(type);
    EXPECT_EQ(task.StorageType(), type);
  }

  task.CompressData(true);
  EXPECT_TRUE(task.CompressData());
  task.CompressData(false);
  EXPECT_FALSE(task.CompressData());

  task.OverwriteFile(true);
  EXPECT_TRUE(task.OverwriteFile());
  task.OverwriteFile(false);
  EXPECT_FALSE(task.OverwriteFile());

  for (auto version = static_cast<size_t>(MdfVersion::Mdf3);
       version <= static_cast<size_t>(MdfVersion::Mdf4_2); ++version) {
    auto ver = static_cast<MdfVersion>(version);
    task.Version(ver);
    EXPECT_EQ(task.Version(), ver);
  }

  EXPECT_FALSE(task.Result());
  task.Result(true);
  EXPECT_TRUE(task.Result());
  task.Result(false);
  EXPECT_FALSE(task.Result());

  EXPECT_FALSE(task.Error());
  task.Error(true);
  EXPECT_TRUE(task.Error());
  task.Error(false);
  EXPECT_FALSE(task.Error());

  constexpr std::string_view kErrorText =
    "All work and no play makes Jack a dull boy";
  task.SaveMessage(std::string(kErrorText));
  ASSERT_EQ(task.MessageList().size(), 1);
  EXPECT_EQ(task.MessageList()[0], kErrorText);

  task.SetMdf4File(true);
  EXPECT_TRUE(task.IsMdf4File());
  task.SetMdf4File(false);
  EXPECT_FALSE(task.IsMdf4File());
}

TEST_F(TestMdfTask, TestValidatingTask) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kError);
  size_t nof_valid = 0;
  size_t nof_invalid = 0;

  for (const auto& [name,file_path] : kMdfList) {
    ValidatingTask task;
    task.SourceFile(file_path);
    path dest_file(kTestDir);
    std::string filename = name + "_valid.mf4";
    dest_file.append(filename);
    task.DestinationFile(dest_file.string());

    task.Run();
    /*
    EXPECT_TRUE(task.Result());
    EXPECT_FALSE(task.Error());
    EXPECT_TRUE(task.ErrorMessage().empty());
    */
    std::cout << "File: " << name <<
      (task.Result() ? " OK" : " ERROR") << std::endl;
    if (!task.Result()) {
      ++nof_invalid;
      for ( const auto& msg : task.MessageList() ) {
        std::cout << "Error: " << msg << std::endl;
      }
    } else {
      ++nof_valid;
    }
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kTrace);
  EXPECT_GT(nof_invalid, 0);
  EXPECT_GT(nof_valid, 0);

}

TEST_F(TestMdfTask, TestSampleRecordObserver) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kError);
  for (const auto& [name,file_path] : kMdfList) {
    const auto filesize = file_size(file_path);
    if (filesize > 1'000'000'000) {
      continue;
    }
    MdfReader reader(file_path);
    ASSERT_TRUE(reader.ReadEverythingButData());
    const auto* file = reader.GetFile();
    ASSERT_TRUE(file != nullptr);
    const auto* header = file->Header();
    ASSERT_TRUE(header != nullptr);
    uint64_t base_time = header->StartTime();
    auto dg_list = header->DataGroups();
    for (IDataGroup* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);
      auto cg_list = data_group->ChannelGroups();
      for (IChannelGroup* channel_group : cg_list) {
        ASSERT_TRUE(channel_group != nullptr);

        if (channel_group->NofSamples() <= 0 ||
          (channel_group->Flags() & CgFlag::VlsdChannel) != 0) {
          continue;
        }
        TestSampleRecordObserver observer(*data_group, *channel_group, base_time);
        reader.ReadData(*data_group);
        EXPECT_EQ(observer.GetSampleIndex() + 1, channel_group->NofSamples());
        std::cout << "File: " << name  << "/" << channel_group->Name() << " OK" << std::endl;
      }
    }
  }

  MdfLogStream::SetLogLevel(MdfLogSeverity::kTrace);
}


TEST_F(TestMdfTask, TestSortingTask) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kError);
  size_t nof_sorted = 0;

  for (const auto& [name,file_path] : kMdfList) {
    if (const auto filesize = file_size(file_path);
      filesize > 1'000'000'000) {
      continue;
    }

    const uint64_t test_start = MdfHelper::NowNs();

    SortingTask task;
    task.SourceFile(file_path);
    path dest_file(kTestDir);
    std::string filename = name + "_sorting.mf4";
    dest_file.append(filename);
    task.DestinationFile(dest_file.string());
    task.SkipIfNoSamples(true);

    task.Run();
    const uint64_t test_stop = MdfHelper::NowNs();
    const double test_time = static_cast<double>(test_stop - test_start) / 1'000'000'000.0;

        /*
    EXPECT_TRUE(task.Result());
    EXPECT_FALSE(task.Error());
    EXPECT_TRUE(task.ErrorMessage().empty());
    */
    std::cout << "File: " << name <<
      (task.Result() ? " OK (" : " ERROR (")
       << test_time << "s)" << std::endl;
    if (!task.Result()) {
      for ( const auto& msg : task.MessageList() ) {
        std::cout << "Error: " << msg << std::endl;
      }
    } else {
      ++nof_sorted;
    }

  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kTrace);
  EXPECT_GT(nof_sorted, 0);
  //EXPECT_GT(nof_non_sorted, 0);

}
}  // namespace mdf::test