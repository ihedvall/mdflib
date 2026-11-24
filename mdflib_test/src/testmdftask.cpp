/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "testmdftask.h"

#include <string>
#include <string_view>
#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "mdf/mdftask.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdffactory.h"

using namespace std::filesystem;
using MdfList = std::map<std::string, std::string>;

namespace {
constexpr std::string_view kMdfDir = "k:/test/mdf";  // Where all source MDF files exist
constexpr std::string_view kDemoFile = "dSPACE_RealTypes.mf4";
MdfList kMdfList; // List of all MDF files

std::string kTestDir; ///< Temp Dir/test/mdf/task";
bool kSkipTest = false;
/*
std::string GetMdfFile(const std::string& name) {
  auto itr = kMdfList.find(name);
  return itr == kMdfList.cend() ? std::string() : itr->second;
}
*/
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
          !kMdfList.contains(stem) && IsMdfFile(fullname.string())) {
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


  auto task = MdfFactory::CreateTask(MdfTaskType::MdfValidatingTask);
  ASSERT_TRUE(task);

  path source_file(kMdfDir);
  source_file.append(kDemoFile);
  const std::string source = source_file.string();
  task->SourceFile(source);
  EXPECT_EQ(task->SourceFile(), source);

  path dest_file(kTestDir);
  dest_file.append(kDemoFile);
  const std::string destination = dest_file.string();
  task->DestinationFile(destination);
  EXPECT_EQ(task->DestinationFile(), destination);

  path temp_file(temp_directory_path());
  temp_file.append(kDemoFile);
  const std::string temporary = temp_file.string();
  task->TempFile(temporary);
  EXPECT_EQ(task->TempFile(), temporary);

  for (auto storage = static_cast<size_t>(MdfStorageType::FixedLengthStorage);
       storage <= static_cast<size_t>(MdfStorageType::MlsdStorage); ++storage) {
    auto type = static_cast<MdfStorageType>(storage);
    task->StorageType(type);
    EXPECT_EQ(task->StorageType(), type);
  }

  task->CompressData(true);
  EXPECT_TRUE(task->CompressData());
  task->CompressData(false);
  EXPECT_FALSE(task->CompressData());

  task->OverwriteFile(true);
  EXPECT_TRUE(task->OverwriteFile());
  task->OverwriteFile(false);
  EXPECT_FALSE(task->OverwriteFile());

  for (auto version = static_cast<size_t>(MdfVersion::Mdf3);
       version <= static_cast<size_t>(MdfVersion::Mdf4_2); ++version) {
    auto ver = static_cast<MdfVersion>(version);
    task->Version(ver);
    EXPECT_EQ(task->Version(), ver);
  }

  EXPECT_FALSE(task->Result());
  task->Result(true);
  EXPECT_TRUE(task->Result());
  task->Result(false);
  EXPECT_FALSE(task->Result());

  EXPECT_FALSE(task->Error());
  task->Error(true);
  EXPECT_TRUE(task->Error());
  task->Error(false);
  EXPECT_FALSE(task->Error());

  constexpr std::string_view kErrorText =
    "All work and no play makes Jack a dull boy";
  task->SaveMessage(std::string(kErrorText));
  ASSERT_EQ(task->MessageList().size(), 1);
  EXPECT_EQ(task->MessageList()[0], kErrorText);

  task->SetMdf4File(true);
  EXPECT_TRUE(task->IsMdf4File());
  task->SetMdf4File(false);
  EXPECT_FALSE(task->IsMdf4File());
}

TEST_F(TestMdfTask, TestValidatingTask) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kError);
  size_t nof_valid = 0;
  size_t nof_invalid = 0;

  for (const auto& [name,file_path] : kMdfList) {
    auto task = MdfFactory::CreateTask(MdfTaskType::MdfValidatingTask);
    ASSERT_TRUE(task);

    task->SourceFile(file_path);
    path dest_file(kTestDir);
    std::string filename = name + "_valid.mf4";
    dest_file.append(filename);
    task->DestinationFile(dest_file.string());

    task->Run();

    std::cout << "File: " << name <<
      (task->Result() ? " OK" : " ERROR") << std::endl;
    if (!task->Result()) {
      ++nof_invalid;
      for ( const auto& msg : task->MessageList() ) {
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

TEST_F(TestMdfTask, TestMdf3Source) {
  path source_file(kTestDir);
  source_file.append("task_source.mdf");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);
    ASSERT_TRUE(writer->Init(source_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel("Time");
    master->Type(ChannelType::Master);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    auto* speed = group->CreateChannel("Speed");
    speed->DataType(ChannelDataType::FloatLe);
    speed->DataBytes(8);
    ASSERT_TRUE(writer->InitMeasurement());
    const uint64_t start = MdfHelper::NowNs();
    writer->StartMeasurement(start);
    for (uint64_t sample = 0; sample < 10; ++sample) {
      speed->SetChannelValue(static_cast<double>(sample));
      writer->SaveSample(*group, start + sample * 1'000'000);
    }
    writer->StopMeasurement(start + 10'000'000);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }

  // The tasks write MF4 and cannot copy an MDF 3 configuration
  for (const auto type : {MdfTaskType::MdfSortingTask,
                          MdfTaskType::MdfCuttingTask}) {
    auto task = MdfFactory::CreateTask(type);
    ASSERT_TRUE(task);
    task->SourceFile(source_file.string());
    path dest_file(kTestDir);
    dest_file.append("task_source_" + std::to_string(static_cast<int>(type))
                     + ".mf4");
    task->DestinationFile(dest_file.string());
    task->Run();
    EXPECT_FALSE(task->Result());
    const auto& messages = task->MessageList();
    EXPECT_TRUE(!messages.empty() && messages.front().find(
        "MDF 3 source files are not supported.") != std::string::npos);
    EXPECT_FALSE(exists(dest_file));
  }
}

TEST_F(TestMdfTask, TestSdText) {
  path source_file(kTestDir);
  source_file.append("sd_text.mf4");
  const std::vector<std::string> text_list = {"ready", "run", "pause",
                                              "bereit", "end"};
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer->Init(source_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup(
        "Group");
    auto* master = group->CreateChannel("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    auto* state = group->CreateChannel("State");
    state->Type(ChannelType::VariableLength);  // Text in an SD block
    state->DataType(ChannelDataType::StringUTF8);
    state->DataBytes(8);
    ASSERT_TRUE(writer->InitMeasurement());
    const uint64_t start = MdfHelper::NowNs();
    writer->StartMeasurement(start);
    for (size_t sample = 0; sample < text_list.size(); ++sample) {
      state->SetChannelValue(text_list[sample]);
      writer->SaveSample(*group, start + sample * 1'000'000'000);
    }
    writer->StopMeasurement(start + 5'000'000'000);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }

  for (const auto type : {MdfTaskType::MdfSortingTask,
                          MdfTaskType::MdfCuttingTask}) {
    const bool cut = type == MdfTaskType::MdfCuttingTask;
    auto task = MdfFactory::CreateTask(type);
    ASSERT_TRUE(task);
    task->SourceFile(source_file.string());
    path dest_file(kTestDir);
    dest_file.append(cut ? "sd_text_cut.mf4" : "sd_text_sort.mf4");
    task->DestinationFile(dest_file.string());
    if (cut) {
      task->MinTime(1.0);
      task->MaxTime(3.0);
    }
    task->Run();
    ASSERT_TRUE(task->Result());

    MdfReader reader(dest_file.string());
    ASSERT_TRUE(reader.ReadEverythingButData());
    auto* data_group = reader.GetFile()->Header()->LastDataGroup();
    ASSERT_TRUE(data_group != nullptr);
    const auto* group = data_group->GetChannelGroup("Group");
    ASSERT_TRUE(group != nullptr);
    const auto* state = group->GetChannel("State");
    ASSERT_TRUE(state != nullptr);
    auto observer = CreateChannelObserver(*data_group, *group, *state);
    ASSERT_TRUE(reader.ReadData(*data_group));
    const size_t first = cut ? 1 : 0;
    const size_t nof_samples = cut ? 3 : text_list.size();
    ASSERT_EQ(observer->NofSamples(), nof_samples);
    for (size_t sample = 0; sample < nof_samples; ++sample) {
      std::string value;
      EXPECT_TRUE(observer->GetEngValue(sample, value));
      EXPECT_EQ(value, text_list[first + sample]);
    }
  }
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

    auto task = MdfFactory::CreateTask(MdfTaskType::MdfSortingTask);
    ASSERT_TRUE(task);

    task->SourceFile(file_path);
    path dest_file(kTestDir);
    std::string filename = name + "_sorting.mf4";
    dest_file.append(filename);
    task->DestinationFile(dest_file.string());
    task->SkipIfNoSamples(true);

    task->Run();
    const uint64_t test_stop = MdfHelper::NowNs();
    const double test_time = static_cast<double>(test_stop - test_start) / 1'000'000'000.0;

    std::cout << "File: " << name <<
      (task->Result() ? " OK (" : " ERROR (")
       << test_time << "s)" << std::endl;
    if (!task->Result()) {
      for ( const auto& msg : task->MessageList() ) {
        std::cout << "Error: " << msg << std::endl;
      }
    } else {
      ++nof_sorted;
    }

  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kTrace);
  EXPECT_GT(nof_sorted, 0);
}


TEST_F(TestMdfTask, TestCutTask) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kError);
  size_t nof_cut = 0;

  for (const auto& [name,file_path] : kMdfList) {
    if (const auto filesize = file_size(file_path);
      filesize > 1'000'000'000) {
      continue;
    }

    MdfReader reader(file_path);
    if (!reader.IsOk()) {
      continue;
    }

    const bool read_config = reader.ReadEverythingButData();
    if (!read_config) {
      continue;
    }
    const auto* file = reader.GetFile();
    if (file == nullptr) {
      continue;
    }
    const IHeader* header = file->Header();
    if (header == nullptr) {
      continue;
    }
    const auto dg_list = header->DataGroups();
    if (dg_list.size()!= 1) {
      continue;
    }
    IDataGroup* data_group = dg_list.front();
    if (data_group == nullptr) {
      continue;
    }
    const auto cg_list = data_group->ChannelGroups();
    if (cg_list.size() != 1) {
      continue;
    }
    const IChannelGroup* channel_group = cg_list.front();
    if (channel_group == nullptr) {
      continue;
    }
    const IChannel* master = channel_group->GetMasterTimeChannel();
    if (master == nullptr) {
      continue;
    }
    auto time_observer = CreateChannelObserver(*data_group,
      *channel_group, *master);
    if (!time_observer) {
      continue;
    }
    const bool read_time = reader.ReadData(*data_group);
    if (!read_time) {
      continue;
    }
    std::vector<double> time_list;
    time_observer->GetEngSamples(time_list);
    const auto nof_samples = time_list.size();
    if (nof_samples < 10) {
      continue;
    }
    // Remove first and last 2 sample i.e. 4 samples
    const double min_time = time_list[2];
    const double max_time = time_list[nof_samples - 3];

    const uint64_t test_start = MdfHelper::NowNs();

    auto task = MdfFactory::CreateTask(MdfTaskType::MdfCuttingTask);
    ASSERT_TRUE(task);

    task->MinTime(min_time);
    task->MaxTime(max_time);
    task->RecalculateTime(true);
    task->SourceFile(file_path);
    path dest_file(kTestDir);
    std::string filename = name + "_cut.mf4";
    dest_file.append(filename);
    task->DestinationFile(dest_file.string());
    task->SkipIfNoSamples(true);

    task->Run();
    const uint64_t test_stop = MdfHelper::NowNs();
    const double test_time = static_cast<double>(test_stop - test_start) / 1'000'000'000.0;

    std::cout << "File: " << name << (task->Result() ? " OK (" : " ERROR (")
       << test_time << "s)" << std::endl;
    if (!task->Result()) {
      for ( const auto& msg : task->MessageList() ) {
        std::cout << "Error: " << msg << std::endl;
      }
    } else {
      ++nof_cut;
    }
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kTrace);
  EXPECT_GT(nof_cut, 0);
}

TEST_F(TestMdfTask, TestMdfToCsvTask) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kError);
  size_t nof_sorted = 0;

  for (const auto& [name,file_path] : kMdfList) {
    if (const auto filesize = file_size(file_path);
      filesize > 1'000'000'000) {
      continue; // We don't convert large files due to test time
    }

    const uint64_t test_start = MdfHelper::NowNs();

    auto task = MdfFactory::CreateTask(MdfTaskType::MdfConvertToCsvTask);
    ASSERT_TRUE(task);

    task->SourceFile(file_path);
    path dest_file(kTestDir);
    std::string filename = name + ".csv";
    dest_file.append(filename);
    task->DestinationFile(dest_file.string());

    task->Run();
    const uint64_t test_stop = MdfHelper::NowNs();
    const double test_time = static_cast<double>(test_stop - test_start) / 1'000'000'000.0;

    std::cout << "File: " << name <<
      (task->Result() ? " OK (" : " ERROR (")
       << test_time << "s)" << std::endl;
    if (!task->Result()) {
      for ( const auto& msg : task->MessageList() ) {
        std::cout << "Error: " << msg << std::endl;
      }
    } else {
      ++nof_sorted;
    }

  }
  MdfLogStream::SetLogLevel(MdfLogSeverity::kTrace);
  EXPECT_GT(nof_sorted, 0);
}

TEST_F(TestMdfTask, TestCsvPrecision) {
  path source_file(kTestDir);
  source_file.append("csv_precision.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer->Init(source_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    auto* current = group->CreateChannel("Current");
    current->DataType(ChannelDataType::FloatLe);
    current->DataBytes(8);
    auto* voltage = group->CreateChannel("Voltage");
    voltage->DataType(ChannelDataType::FloatLe);
    voltage->DataBytes(8);
    voltage->Decimals(3);
    auto* speed = group->CreateChannel("Speed");
    speed->DataType(ChannelDataType::FloatLe);
    speed->DataBytes(4);
    ASSERT_TRUE(writer->InitMeasurement());
    const uint64_t start = MdfHelper::NowNs();
    writer->StartMeasurement(start);
    current->SetChannelValue(1.25e-7);
    voltage->SetChannelValue(12.3456789);
    speed->SetChannelValue(1.8137907F);
    writer->SaveSample(*group, start + 183'096);
    writer->StopMeasurement(start + 183'096);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }

  auto task = MdfFactory::CreateTask(MdfTaskType::MdfConvertToCsvTask);
  ASSERT_TRUE(task);
  task->SourceFile(source_file.string());
  path dest_file(kTestDir);
  dest_file.append("csv_precision.csv");
  task->DestinationFile(dest_file.string());
  task->Run();
  ASSERT_TRUE(task->Result());

  std::ifstream csv(dest_file);
  std::string row;
  std::getline(csv, row);  // Header
  std::getline(csv, row);
  std::vector<std::string> fields;
  std::istringstream line(row);
  for (std::string field; std::getline(line, field, ';');) {
    fields.emplace_back(field);
  }
  ASSERT_EQ(fields.size(), 4) << row;
  // The CSV keeps all digits, also when the channel has a display precision
  EXPECT_EQ(std::stod(fields[0]), 183'096 / 1e9) << row;
  EXPECT_EQ(std::stod(fields[1]), 1.25e-7) << row;
  EXPECT_EQ(std::stod(fields[2]), 12.3456789) << row;
  EXPECT_EQ(fields[3], "1.8137907") << row;
}

TEST_F(TestMdfTask, TestCsvQuotes) {
  path source_file(kTestDir);
  source_file.append("csv_quotes.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer->Init(source_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    auto* state = group->CreateChannel("State \"A\"");
    state->DataType(ChannelDataType::StringUTF8);
    state->DataBytes(16);
    state->Unit("V \"rms\"");
    ASSERT_TRUE(writer->InitMeasurement());
    const uint64_t start = MdfHelper::NowNs();
    writer->StartMeasurement(start);
    state->SetChannelValue(std::string("said \"on\""));
    writer->SaveSample(*group, start);
    writer->StopMeasurement(start);
    ASSERT_TRUE(writer->FinalizeMeasurement());
  }

  auto task = MdfFactory::CreateTask(MdfTaskType::MdfConvertToCsvTask);
  ASSERT_TRUE(task);
  task->SourceFile(source_file.string());
  path dest_file(kTestDir);
  dest_file.append("csv_quotes.csv");
  task->DestinationFile(dest_file.string());
  task->Run();
  ASSERT_TRUE(task->Result());

  std::ifstream csv(dest_file);
  std::string header;
  std::string row;
  std::getline(csv, header);
  std::getline(csv, row);
  // A quote inside a quoted field is written twice
  EXPECT_NE(header.find("\"State \"\"A\"\" [V \"\"rms\"\"]\""), std::string::npos)
      << header;
  EXPECT_NE(row.find("\"said \"\"on\"\"\""), std::string::npos) << row;
}
}  // namespace mdf::test