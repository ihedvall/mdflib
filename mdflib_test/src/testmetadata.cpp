/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <chrono>
#include "testmetadata.h"

#include <memory>
#include <filesystem>


#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "md4block.h"
#include "hd4block.h"
#include "fh4block.h"
#include "cn4block.h"
#include "sr4block.h"

#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdflogstream.h"

using namespace mdf;
using namespace mdf::detail;
using namespace util::log;
using namespace std::filesystem;
using namespace std::chrono_literals;
using namespace util::time;

namespace {

std::string kTestRootDir;  ///< Test root directory
std::string kTestDir;      ///<  <Test Root Dir>/mdf/metadata";
bool kSkipTest = false;

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogFunc(const MdfLocation& location, MdfLogSeverity severity,
             const std::string& text) {
  const auto& log_config = LogConfig::Instance();
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
void NoLog(const MdfLocation& location, mdf::MdfLogSeverity severity,
           const std::string& text) {}
} // end namespace

namespace mdf::test {

void TestMetaData::SetUpTestSuite() {

  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Not deleted


    // Log to a file as this file is used as attachment file
    auto& log_config = LogConfig::Instance();
    log_config.RootDir(kTestRootDir);
    log_config.BaseName("mdf_metadata");
    log_config.Type(LogType::LogToFile);
    log_config.CreateDefaultLogger();

    remove(log_config.GetLogFile()); // Remove any old log files

    // Connect MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);


    // Add console logging
    log_config.AddLogger("Console",LogType::LogToConsole, {});
    LOG_TRACE() << "Log file created. File: " << log_config.GetLogFile();

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("metadata");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      LOG_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();


    LOG_TRACE() << "Created the test directory. Dir: " << kTestDir;
  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestMetaData::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();

}

TEST_F(TestMetaData, HDComment) {
  auto md4 = std::make_unique<detail::Md4Block>();
  auto* meta_data = md4.get();
  meta_data->InitMd("HDcomment");
  std::cout << meta_data->XmlSnippet() << std::endl;
  meta_data->StringProperty("TX", "Comments");
  EXPECT_TRUE(meta_data->StringProperty("TX") == "Comments");
  EXPECT_TRUE(meta_data->StringProperty("tx") == "Comments");

  ETag olle;
  olle.Name("Olle");
  olle.Description("Olle description");
  olle.Type("float");
  olle.Value(1.23);
  olle.Unit("m");
  meta_data->CommonProperty(olle);
  std::cout << meta_data->XmlSnippet() << std::endl;

  ETag olle_list;
  olle_list.Name("olle_list");

  ETag olle1 = olle;
  olle1.Name("olle1");
  olle_list.AddTag(olle1);
  meta_data->CommonProperty(olle_list);
  std::cout << meta_data->XmlSnippet() << std::endl;

  auto olle2 = meta_data->CommonProperty("Olle");
  EXPECT_TRUE(olle2.Name() == olle.Name());
  EXPECT_TRUE(olle2.Description() == olle.Description());

  const auto olle2_list = meta_data->CommonProperties();
  EXPECT_EQ(olle2_list.size(), 2);
}

TEST_F(TestMetaData, Hd4Block) {
  Hd4Block hd4;
  hd4.Author("Ingemar Hedvall");
  hd4.Department("Home");

  EXPECT_STREQ(hd4.Author().c_str(), "Ingemar Hedvall");
  EXPECT_STREQ(hd4.Department().c_str(), "Home");

  const auto* md4 = hd4.MetaData();
  ASSERT_TRUE(md4 != nullptr);
  std::cout << md4->XmlSnippet() << std::endl;
}

TEST_F(TestMetaData, Fh4Block) {
  Fh4Block fh4;

  fh4.Time(1234);
  EXPECT_EQ(fh4.Time(), 1234);

  fh4.Description("Comments");
  fh4.ToolName("Tester");

  EXPECT_STREQ(fh4.Description().c_str(), "Comments");
  EXPECT_STREQ(fh4.ToolName().c_str(), "Tester");

  const auto* md4 = fh4.MetaData();
  ASSERT_TRUE(md4 != nullptr);
  std::cout << md4->XmlSnippet() << std::endl;
}

TEST_F(TestMetaData, OdsMetaData) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("ods_metadata.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  header->Author("Road Runner");
  header->Department("ACME Corporation");
  header->Project("Explosive Tennis Balls");
  header->Subject("Coyote Equipment");
  header->Description("What can go wrong!");

  auto* meta_data = header->MetaData();
  ASSERT_TRUE(meta_data != nullptr);

  ETag engine_code;
  engine_code.Name("Engine_Type");
  engine_code.Description("Engine Type");
  engine_code.DataType(ETagDataType::StringType);
  engine_code.Value("DIESEL");
  meta_data->CommonProperty(engine_code);

  ETag test_id;
  test_id.Name("Test_Id");
  test_id.Description("Test Identity");
  test_id.DataType(ETagDataType::StringType);
  test_id.Value("Ball Test");
  meta_data->CommonProperty(test_id);

  ETag test_time;
  test_time.Name("Test_Time");
  test_time.Description("Test Time");
  test_time.DataType(ETagDataType::DateTimeType);
  test_time.Value(MdfHelper::NsToLocalIsoTime(MdfHelper::NowNs()));
  meta_data->CommonProperty(test_time);

  ETag test_result;
  test_result.Name("Test_Result");
  test_result.Description("Test Result");
  test_result.DataType(ETagDataType::BooleanType);
  test_result.Value(false);
  meta_data->CommonProperty(test_result);

  std::cout << "ODS META-DATA" << std::endl
      << meta_data->XmlSnippet() << std::endl;


  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group = data_group->CreateChannelGroup();
  group->Name("Unsigned");

  auto* master = group->CreateChannel();
  master->Name("Master");
  master->Type(ChannelType::Master);
  master->Sync(ChannelSyncType::Time);
  master->DataType(ChannelDataType::FloatLe);
  master->DataBytes(8);

  auto* channel = group->CreateChannel();
  channel->Name("Intel64");
  channel->Type(ChannelType::FixedLength);
  channel->Sync(ChannelSyncType::None);
  channel->DataType(ChannelDataType::UnsignedIntegerLe);
  channel->DataBytes(8);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  uint64_t value;
  for (size_t sample = 0; sample < 100; ++sample) {
    value = static_cast<uint64_t>(sample);
    channel->SetChannelValue(value);
    writer->SaveSample(*group,tick_time);
    tick_time += 1'000'000'000; // Every second
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (size_t sample = 0; sample < 100; ++sample) {
      uint64_t channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid) << "Sample: " << sample << ", Name: " << observer->Name();
      EXPECT_EQ(channel_value, static_cast<uint64_t>(sample))
          << observer->Name();
    }
  }

}

}  // namespace mdf::test
