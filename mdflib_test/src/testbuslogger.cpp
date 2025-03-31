/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "testbuslogger.h"

#include <filesystem>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/mdflogstream.h"
#include "mdf/mdfhelper.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdfreader.h"
#include "mdf/ifilehistory.h"
#include "mdf/linmessage.h"
#include "mdf/ethmessage.h"
#include "mdf/idatagroup.h"

using namespace util::log;
using namespace std::filesystem;
using namespace util::time;
namespace {

std::string kTestRootDir; ///< Test root directory
std::string kTestDir; ///<  <Test Root Dir>/mdf/bus";
bool kSkipTest = false;

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogFunc(const MdfLocation& , mdf::MdfLogSeverity severity,
             const std::string& text) {
  const auto &log_config = LogConfig::Instance();
  LogMessage message;
  message.message = text;
  message.severity = static_cast<LogSeverity>(severity);

  log_config.AddLogMessage(message);
}
/**
 * Function that stops logging of MDF logs
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void NoLog(const MdfLocation& , mdf::MdfLogSeverity ,
           const std::string& ) {
}

}

namespace mdf::test {



void TestBusLogger::SetUpTestSuite() {

  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Not deleted


    // Log to the console instead of as normally to a file
    auto& log_config = LogConfig::Instance();
    log_config.Type(LogType::LogToConsole);
    log_config.CreateDefaultLogger();

    // Connect MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);

    LOG_TRACE() << "Log file created. File: " << log_config.GetLogFile();

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("bus");
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

void TestBusLogger::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}


TEST_F(TestBusLogger, Mdf4CanSdStorage ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("can_sd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::FixedLengthStorage); // Variable length to SD
  writer->MaxLength(8); // No meaning in this type of storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration()); // Creates all DG/CG/CN
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < 10; ++sample) {
    // Assigned some dummy data
    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 10);
  }

}

TEST_F(TestBusLogger, Mdf4VlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("can_vlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(20);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    if (sample < 5) {
      SampleRecord temp;
      msg.ToRaw(MessageType::CAN_DataFrame,temp, 8, true);
      std::cout << "Sample: " << sample
                << ", Size: " << temp.vlsd_buffer.size() << std::endl;
    }

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);


  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 6);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();



  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);
    if (observer->Name() == "CAN_DataFrame" ) {
      for (uint64_t sample1 = 0;
           sample1 < 5 && sample1 < observer->NofSamples();
           ++sample1) {
        std::vector<uint8_t> frame_data;
        observer->GetEngValue(sample1,frame_data);
        std::cout << "Sample: " << sample1
                  << ", Size: " << frame_data.size() << std::endl;
      }
    }
  }

}

TEST_F(TestBusLogger, Mdf4MlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("can_mlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::MlsdStorage);
  writer->MaxLength(8);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (unique_list.find(name) == unique_list.cend()) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }

}

TEST_F(TestBusLogger, Mdf4CompressedCanSdStorage ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("compressed_can_sd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::FixedLengthStorage); // Variable length to SD
  writer->MaxLength(8); // No meaning in this type of storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration()); // Creates all DG/CG/CN
  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < 10; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 10);
  }

}

TEST_F(TestBusLogger, Mdf4CompressedVlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("compressed_can_vlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8);

  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(true);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 6);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);

  }

}

TEST_F(TestBusLogger, Mdf4CompressedMlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("compressed_can_mlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::MlsdStorage);
  writer->MaxLength(8);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(true);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);
  }

}

TEST_F(TestBusLogger, Mdf4SampleObserver ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("sample_observer.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::FixedLengthStorage); // Variable length to SD
  writer->MaxLength(8); // No meaning in this type of storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration()); // Creates all DG/CG/CN
  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < 1000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* header1 = reader.GetHeader();
  auto* last_dg1 = header1->LastDataGroup();
  ASSERT_TRUE(last_dg1 != nullptr);

  const auto* channel_group1 = last_dg1->GetChannelGroup("CAN_DataFrame");
  ASSERT_TRUE(channel_group1 != nullptr);

  const auto* channel1 = channel_group1->GetChannel("CAN_DataFrame.DataBytes");
  ASSERT_TRUE(channel1 != nullptr);

  uint64_t nof_sample_read = 0; // Testing abort of reading just 10 samples
  ISampleObserver sample_observer(*last_dg1);
  sample_observer.DoOnSample = [&] (uint64_t sample1, uint64_t record_id,
                                   const std::vector<uint8_t>& record ) -> bool {
    bool valid = true;
    std::string values;
    if (channel1->RecordId() == record_id) {
      valid = sample_observer.GetEngValue(*channel1,sample1,
                                          record, values );
      std::cout << "Sample: " << sample1
                << ", Record: " << record_id
                << ", Values: " << values << std::endl;
      ++nof_sample_read;
      if (sample1 >= 9) {
        return false;
      }

    }
    EXPECT_TRUE(valid);
    return true;
  };
  reader.ReadData(*last_dg1);
  EXPECT_EQ(nof_sample_read, 10); // The read should be interrupted after
                                   // 10 samples
  sample_observer.DetachObserver();
  reader.Close();
}

TEST_F(TestBusLogger, TestLinMessage) {
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  ASSERT_TRUE(writer);

  LinMessage msg(*writer);

  msg.BusChannel(5);
  EXPECT_EQ(msg.BusChannel(), 5);

  msg.LinId(0x3F);
  EXPECT_EQ(msg.LinId(), 0x3F);

  msg.Dir(true);
  EXPECT_TRUE(msg.Dir());

  msg.ReceivedDataByteCount(8);
  EXPECT_EQ(msg.ReceivedDataByteCount(), 8);

  msg.DataLength(7);
  EXPECT_EQ(msg.DataLength(), 7);

  std::vector<uint8_t> data = {0,1,2,3,4,5};
  msg.DataBytes(data);
  EXPECT_EQ(msg.DataBytes(), data);
  EXPECT_EQ(msg.DataLength(), 6);

  msg.Checksum(0xAA);
  EXPECT_EQ(msg.Checksum(),0xAA);

  msg.ChecksumModel(LinChecksumModel::Enhanced);
  EXPECT_EQ(msg.ChecksumModel(),LinChecksumModel::Enhanced);

  msg.StartOfFrame(1000);
  EXPECT_EQ(msg.StartOfFrame(),1000);

  msg.Baudrate(1200.0F);
  EXPECT_FLOAT_EQ(msg.Baudrate(),1200.0F);

  msg.ResponseBaudrate(2400.0F);
  EXPECT_FLOAT_EQ(msg.ResponseBaudrate(),2400.0F);

  msg.BreakLength(2000);
  EXPECT_EQ(msg.BreakLength(),2000);

  msg.DelimiterBreakLength(3000);
  EXPECT_EQ(msg.DelimiterBreakLength(),3000);

  msg.ExpectedDataByteCount(3);
  EXPECT_EQ(msg.ExpectedDataByteCount(),3);

  msg.TypeOfLongDominantSignal(LinTypeOfLongDominantSignal::CyclicReport);
  EXPECT_EQ(msg.TypeOfLongDominantSignal(),LinTypeOfLongDominantSignal::CyclicReport);

  msg.TotalSignalLength(1900);
  EXPECT_EQ(msg.TotalSignalLength(),1900);
}

TEST_F(TestBusLogger, Mdf4LinConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("lin_mlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::LIN);
  writer->StorageType(MdfStorageType::MlsdStorage);
  writer->MaxLength(8);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  IDataGroup* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  IChannelGroup* lin_frame = last_dg->GetChannelGroup("LIN_Frame");
  ASSERT_TRUE(lin_frame != nullptr);

  IChannelGroup* lin_wake_up = last_dg->GetChannelGroup("LIN_WakeUp");
  ASSERT_TRUE(lin_wake_up != nullptr);

  IChannelGroup* lin_checksum_error = last_dg->GetChannelGroup("LIN_ChecksumError");
  ASSERT_TRUE(lin_checksum_error != nullptr);

  IChannelGroup* lin_transmission_error = last_dg->GetChannelGroup("LIN_TransmissionError");
  ASSERT_TRUE(lin_transmission_error != nullptr);

  IChannelGroup* lin_sync_error = last_dg->GetChannelGroup("LIN_SyncError");
  ASSERT_TRUE(lin_sync_error != nullptr);

  IChannelGroup* lin_receive_error = last_dg->GetChannelGroup("LIN_ReceiveError");
  ASSERT_TRUE(lin_receive_error != nullptr);

  IChannelGroup* lin_spike = last_dg->GetChannelGroup("LIN_Spike");
  ASSERT_TRUE(lin_spike != nullptr);

  IChannelGroup* lin_long_dom = last_dg->GetChannelGroup("LIN_LongDom");
  ASSERT_TRUE(lin_long_dom != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    LinMessage msg(*writer);
    msg.LinId(12);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveLinMessage(*lin_frame, tick_time, msg);
    writer->SaveLinMessage(*lin_wake_up, tick_time, msg);
    writer->SaveLinMessage(*lin_checksum_error, tick_time, msg);
    writer->SaveLinMessage(*lin_transmission_error, tick_time, msg);
    writer->SaveLinMessage(*lin_sync_error, tick_time, msg);
    writer->SaveLinMessage(*lin_receive_error, tick_time, msg);
    writer->SaveLinMessage(*lin_spike, tick_time, msg);
    writer->SaveLinMessage(*lin_long_dom, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 8);
    CreateChannelObserverForDataGroup(*dg4, observer_list);
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (unique_list.find(name) == unique_list.cend()) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }
}

TEST_F(TestBusLogger, Mdf4LinMandatory) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("lin_mandatory.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::LIN);
  writer->StorageType(MdfStorageType::MlsdStorage);
  writer->MaxLength(8);
  writer->MandatoryMembersOnly(true);
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  EXPECT_TRUE(writer->CreateBusLogConfiguration());

  IDataGroup* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  IChannelGroup* lin_frame = last_dg->GetChannelGroup("LIN_Frame");
  ASSERT_TRUE(lin_frame != nullptr);

  IChannelGroup* lin_wake_up = last_dg->GetChannelGroup("LIN_WakeUp");
  ASSERT_TRUE(lin_wake_up != nullptr);

  IChannelGroup* lin_checksum_error = last_dg->GetChannelGroup("LIN_ChecksumError");
  ASSERT_TRUE(lin_checksum_error != nullptr);

  IChannelGroup* lin_transmission_error = last_dg->GetChannelGroup("LIN_TransmissionError");
  ASSERT_TRUE(lin_transmission_error != nullptr);

  IChannelGroup* lin_sync_error = last_dg->GetChannelGroup("LIN_SyncError");
  ASSERT_TRUE(lin_sync_error != nullptr);

  IChannelGroup* lin_receive_error = last_dg->GetChannelGroup("LIN_ReceiveError");
  ASSERT_TRUE(lin_receive_error != nullptr);

  IChannelGroup* lin_spike = last_dg->GetChannelGroup("LIN_Spike");
  ASSERT_TRUE(lin_spike != nullptr);

  IChannelGroup* lin_long_dom = last_dg->GetChannelGroup("LIN_LongDom");
  ASSERT_TRUE(lin_long_dom != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    LinMessage msg(*writer);
    msg.LinId(12);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveLinMessage(*lin_frame, tick_time, msg);
    writer->SaveLinMessage(*lin_wake_up, tick_time, msg);
    writer->SaveLinMessage(*lin_checksum_error, tick_time, msg);
    writer->SaveLinMessage(*lin_transmission_error, tick_time, msg);
    writer->SaveLinMessage(*lin_sync_error, tick_time, msg);
    writer->SaveLinMessage(*lin_receive_error, tick_time, msg);
    writer->SaveLinMessage(*lin_spike, tick_time, msg);
    writer->SaveLinMessage(*lin_long_dom, tick_time, msg);

    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 8);
    CreateChannelObserverForDataGroup(*dg4, observer_list);
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);
    const auto& valid_list = observer->GetValidList();
    const bool all_valid =  std::all_of(valid_list.cbegin(), valid_list.cend(),
                                       [] (const bool& valid) -> bool {
      return valid;
    });

    EXPECT_TRUE(all_valid) << observer->Name();

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (unique_list.find(name) == unique_list.cend()) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }

}

TEST_F(TestBusLogger, TestEthMessage) {
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  ASSERT_TRUE(writer);

  EthMessage msg(*writer);

  msg.BusChannel(5);
  EXPECT_EQ(msg.BusChannel(), 5);

  msg.Dir(true);
  EXPECT_TRUE(msg.Dir());

  const uint8_t source_addr[6] = {1,2,3,4,5,6};
  const uint8_t* source_temp = source_addr;
  msg.Source(source_temp);
  const auto& source = msg.Source();
  for (size_t index1 = 0; index1 < 6; ++index1) {
    EXPECT_EQ(source[index1], source_addr[index1]);
  }

  const uint8_t dest_addr[6] = {2,3,4,5,6,7};
  const uint8_t* dest_temp = dest_addr;
  msg.Destination(dest_temp);
  const auto& dest = msg.Destination();
  for (size_t index2 = 0; index2 < 6; ++index2) {
    EXPECT_EQ(dest[index2], dest_addr[index2]);
  }

  msg.ReceivedDataByteCount(8);
  EXPECT_EQ(msg.ReceivedDataByteCount(), 8);

  msg.DataLength(7);
  EXPECT_EQ(msg.DataLength(), 7);

  std::vector<uint8_t> data = {0,1,2,3,4,5};
  msg.DataBytes(data);
  EXPECT_EQ(msg.DataBytes(), data);
  EXPECT_EQ(msg.DataLength(), 6);

}

TEST_F(TestBusLogger, Mdf4EthConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("eth_vlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  EXPECT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::Ethernet);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8); // Not needed for SD and VLSD storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  IDataGroup* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  IChannelGroup* eth_frame = last_dg->GetChannelGroup("ETH_Frame");
  ASSERT_TRUE(eth_frame != nullptr);

  IChannelGroup* eth_checksum_error = last_dg->GetChannelGroup("ETH_ChecksumError");
  ASSERT_TRUE(eth_checksum_error != nullptr);

  IChannelGroup* eth_length_error = last_dg->GetChannelGroup("ETH_LengthError");
  ASSERT_TRUE(eth_length_error != nullptr);

  IChannelGroup* eth_receive_error = last_dg->GetChannelGroup("ETH_ReceiveError");
  ASSERT_TRUE(eth_receive_error != nullptr);

  EXPECT_TRUE(writer->InitMeasurement());
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    EthMessage msg(*writer);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveEthMessage(*eth_frame, tick_time, msg);
    writer->SaveEthMessage(*eth_checksum_error, tick_time, msg);
    writer->SaveEthMessage(*eth_length_error, tick_time, msg);
    writer->SaveEthMessage(*eth_receive_error, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 8);
    CreateChannelObserverForDataGroup(*dg4, observer_list);
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (unique_list.find(name) == unique_list.cend()) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }
}

TEST_F(TestBusLogger, Mdf4EthMandatory) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("eth_mandatory.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  if (auto* md4_file = writer->GetFile();
      md4_file != nullptr ) {
    md4_file->MinorVersion(11);
  }

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::Ethernet);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8);
  writer->MandatoryMembersOnly(true);
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  EXPECT_TRUE(writer->CreateBusLogConfiguration());

  IDataGroup* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  IChannelGroup* eth_frame = last_dg->GetChannelGroup("ETH_Frame");
  ASSERT_TRUE(eth_frame != nullptr);

  IChannelGroup* eth_checksum_error = last_dg->GetChannelGroup("ETH_ChecksumError");
  ASSERT_TRUE(eth_checksum_error != nullptr);

  IChannelGroup* eth_length_error = last_dg->GetChannelGroup("ETH_LengthError");
  ASSERT_TRUE(eth_length_error != nullptr);

  IChannelGroup* eth_receive_error = last_dg->GetChannelGroup("ETH_ReceiveError");
  ASSERT_TRUE(eth_receive_error != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    EthMessage msg(*writer);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveEthMessage(*eth_frame, tick_time, msg);
    writer->SaveEthMessage(*eth_checksum_error, tick_time, msg);
    writer->SaveEthMessage(*eth_length_error, tick_time, msg);
    writer->SaveEthMessage(*eth_receive_error, tick_time, msg);

    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 5);
    CreateChannelObserverForDataGroup(*dg4, observer_list);
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);
    const auto& valid_list = observer->GetValidList();
    const bool all_valid =  std::all_of(valid_list.cbegin(), valid_list.cend(),
                                       [] (const bool& valid) -> bool {
                                         return valid;
                                       });

    EXPECT_TRUE(all_valid) << observer->Name();

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (unique_list.find(name) == unique_list.cend()) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }

}


}  // namespace mdf::test