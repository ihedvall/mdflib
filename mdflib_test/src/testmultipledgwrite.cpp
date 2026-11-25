/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "testmultipledgwrite.h"
#include <chrono>
#include <filesystem>
#include <thread>

#include "util/logconfig.h"
#include "util/logstream.h"

#include "mdf/mdflogstream.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/ichannel.h"
#include "mdf/mdfreader.h"

using namespace util::log;
using namespace std::filesystem;
using namespace std::chrono_literals;
using namespace std::this_thread;

namespace {

std::string kTestDir; ///<  <Test Root Dir>/mdf/write;
bool kSkipTest = false;

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogFunc(const MdfLocation& location, mdf::MdfLogSeverity severity,
             const std::string& text) {
  const auto &log_config = LogConfig::Instance();

  // Copy MdfLocation to std::source_location is still a problem.
  // Adding it to the text for the time being.
  std::ostringstream temp;
  temp << text << " [" << location.file
       << "::" << location.function
       << ":" << location.line << "]";
  LogMessage message;
  message.message = temp.str();
  message.severity = static_cast<LogSeverity>(severity);

  log_config.AddLogMessage(message);
}
/**
 * Function that stops logging of MDF logs
 */
void NoLog(const MdfLocation& , mdf::MdfLogSeverity,
           const std::string& ) {
}

}  // namespace


namespace mdf::test {
void TestMultipleDgWrite::SetUpTestSuite() {

    try {
    // The log output is sent to the console while the test files are
    // kept until next run.
    // By this the test files may be inspected.

  // Log toe console
    auto& log_config = LogConfig::Instance();
    log_config.Type(LogType::LogToConsole);
    log_config.CreateDefaultLogger();

    // Connect the MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    temp_dir.append("mdf");
    temp_dir.append("multiple_dg");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      LOG_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

    LOG_TRACE() << "Created the test directory. Dir: " << kTestDir;
    kSkipTest = false;

  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestMultipleDgWrite::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestMultipleDgWrite, Mdf3SingleDg) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  size_t max_sample = 10'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("single_dg.mf3");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("ACME Road Runner Company");
    header->Description("Testing Single DG Measurement");
    header->Project("MDF3 Write Multiple (Sorted) File");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    auto old_f_size = file_size(mdf_file);
    size_t file_save = 0;
    size_t sample;
    for (sample = 0; sample < max_sample && file_save <= 1; ++sample) {
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value =  static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            if (channel->Type() == ChannelType::Master) {
              continue;
            }
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*channel_group, MdfHelper::NowNs());
        }
        yield();
        // sleep_for(2ms);
        auto f_size = file_size(mdf_file);
        if (f_size >old_f_size ) {
          ++file_save;
          old_f_size = f_size;
        }
      }
    }
    max_sample = sample;
    EXPECT_GT(file_save, 1);
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

    LOG_TRACE() << "File Saved: " << file_save << " times.";
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 1);

    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GT(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            auto expected_value = static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }

    }
  }

}


TEST_F(TestMultipleDgWrite, Mdf3MultiDg) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 150;
  path mdf_file(kTestDir);
  mdf_file.append("multi_dg.mf3");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF3 Write Multiple DG");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
        yield();
        //sleep_for(100ms);
      }
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GT(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4MultiDg) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("multi_dg.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF4 Write Multiple DG");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
        yield();
        //sleep_for(0ms);
      }
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GE(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4ConverterMultiDgCompress) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("converter_multi_dg_compress.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
    ASSERT_TRUE(writer != nullptr);

    writer->PreTrigTime(0.0);
    writer->CompressData(true);
    writer->SavePeriodic(true);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF4 Write Multiple DG");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
      }
      yield();
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

    const uint64_t write_time = (stop_time - start_time)/ 1'000'000;
    LOG_TRACE() << "Converter Write Time [ms]: " << write_time;
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GE(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4ConverterDgAfterDgCompress) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  // Each group is below the queue limit and is saved when the next one starts.
  constexpr size_t max_sample = 100'000;
  path mdf_file(kTestDir);
  mdf_file.append("converter_dg_after_dg_compress.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
    ASSERT_TRUE(writer != nullptr);

    writer->PreTrigTime(0.0);
    writer->CompressData(true);
    writer->SavePeriodic(true);
    writer->Init(mdf_file.string());

    std::vector<IChannelGroup*> group_list;
    for (size_t dg_index = 0; dg_index < 3; ++dg_index) {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);
      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      auto* channel = channel_group->CreateChannel();
      channel->Name("Value");
      channel->Type(ChannelType::FixedLength);
      channel->DataType(ChannelDataType::FloatLe);
      channel->DataBytes(8);
      group_list.push_back(channel_group);
    }

    writer->InitMeasurement();
    writer->StartMeasurement(MdfHelper::NowNs());
    const auto start_size = file_size(mdf_file);
    uint32_t dg_count = 0;
    for (auto* channel_group : group_list) {
      for (size_t sample = 0; sample < max_sample; ++sample) {
        const auto value = (10.0 * dg_count) + static_cast<double>(sample);
        for (auto* channel : channel_group->Channels()) {
          channel->SetChannelValue(value);
        }
        writer->SaveSample(*channel_group, MdfHelper::NowNs());
      }
      ++dg_count;
    }
    // The groups that are left are saved before the stop.
    for (size_t wait = 0; wait < 100 && file_size(mdf_file) <= start_size;
         ++wait) {
      sleep_for(50ms);
    }
    EXPECT_GT(file_size(mdf_file), start_size);
    writer->StopMeasurement(MdfHelper::NowNs());
    writer->FinalizeMeasurement();
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());
    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);

    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 3);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 2);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        if (observer->IsMaster()) {
          continue;
        }
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          const double expected_value =
              (10.0 * dg_count) + static_cast<double>(sample);
          double value = 0.0;
          const bool valid = observer->GetEngValue(sample, value);
          EXPECT_TRUE(valid) << "Sample: " << sample;
          EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
        }
      }
      ++dg_count;
    }
  }
}

TEST_F(TestMultipleDgWrite, Mdf4ConverterMultiNoCompressDg) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("converter_multi_dg_no_compress.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
    ASSERT_TRUE(writer != nullptr);

    writer->PreTrigTime(0.0);
    writer->CompressData(false);
    writer->SavePeriodic(false);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement with no Compression");
    header->Project("MDF4 Write Multiple DG No Compress");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
      }
      yield();
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

    const uint64_t write_time = (stop_time - start_time)/ 1'000'000;
    LOG_TRACE() << "Converter Write Time [ms]: " << write_time;
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples() && sample < 10; ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GE(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4WriteMultiDgSd) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("multi_dg_sd.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF4 Write Multiple DG with SD block");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::VariableLength);
        channel->DataType(ChannelDataType::StringUTF8);
        channel->DataBytes(8);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      uint64_t sample_time = start_time + (sample * 100'000'000);
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          float value = (10.0F * dg_count) + static_cast<float>(sample);
          for (auto* channel : channel_group->Channels()) {
            switch (channel->Type()) {
              case ChannelType::Master:
                // No need to save sample time
                break;

              case ChannelType::VariableLength:
                channel->SetChannelValue(std::to_string(value));
                break;

              default:
                channel->SetChannelValue(value);
                break;
            }
          }
          writer->SaveSample(*data_group, *channel_group, sample_time);
          ++dg_count;
        }
        yield();
      }
    }
    const uint64_t stop_time = start_time + (max_sample * 100'000'000);;
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        const IChannel& channel = observer->Channel();
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          switch (channel.Type()) {
            case ChannelType::Master: {
              float sample_time = 0.0;
              const bool valid = observer->GetEngValue(sample, sample_time);
              ASSERT_TRUE(valid) << "Master Sample: " << sample;
              ASSERT_GE(sample_time, old_time) << "Master Sample: " << sample;
              old_time = sample_time;
              break;
            }

            case ChannelType::VariableLength:  {
              std::string expected_value = std::to_string(
                  (10.0F * dg_count) + static_cast<float>(sample));
              std::string value;
              const bool valid = observer->GetEngValue(sample, value);
              ASSERT_TRUE(valid) << "String Sample: " << sample;
              ASSERT_EQ(expected_value, value) << "String Sample: " << sample;
              break;
            }

            default:  {
              float expected_value =
                  (10.0F * dg_count) + static_cast<float>(sample);
              double value = 0.0;
              const bool valid = observer->GetEngValue(sample, value);
              ASSERT_TRUE(valid) << "Float Sample: " << sample;
              ASSERT_FLOAT_EQ(expected_value, value) << "Float Sample: " << sample;
              break;
            }
          }
        }

      }
      ++dg_count;
    }
  }
}


}  // namespace mdf::test