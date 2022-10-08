/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "testwrite.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

#include "mdf/iattachment.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/ievent.h"
#include "mdf/ifilehistory.h"
#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

namespace {

constexpr std::string_view kLogRootDir = "o:/test";
constexpr std::string_view kLogFile = "mdf_write.log";
constexpr std::string_view kTestDir = "o:/test/mdf/write";

bool kSkipTest = false;
}  // namespace

using namespace std::this_thread;
using namespace std::chrono_literals;
using namespace util::log;
using namespace util::time;
using namespace std::filesystem;
using namespace mdf;
namespace mdf::test {

void TestWrite::SetUpTestSuite() {
  auto& log_config = LogConfig::Instance();
  log_config.RootDir(kLogRootDir.data());
  log_config.BaseName(kLogFile.data());
  log_config.Type(util::log::LogType::LogToFile);
  log_config.CreateDefaultLogger();

  LOG_DEBUG() << "Created the log file";
  try {
    std::error_code err;
    remove_all(kTestDir, err);
    create_directories(kTestDir);
    for (size_t log = 0; log < 60'000; ++log) {
      if (exists(log_config.GetLogFile())) {
        break;
      }
      std::this_thread::sleep_for(1ms);
    }
  } catch (const std::exception& error) {
    LOG_ERROR() << "Failed to create directories. Error: " << error.what();
    kSkipTest = true;
  }
}

void TestWrite::TearDownTestSuite() {
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestWrite, Mdf3WriteHD) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("hd3.mf3");
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");
  header->Department("Home Alone");
  header->Description("Testing Header");
  header->Project("Mdf3WriteHD");
  header->StartTime(TimeStampToNs());
  header->Subject("PXY");

  for (size_t dg_index = 0; dg_index < 2; ++dg_index) {
    auto* dg3 = writer->CreateDataGroup();
    ASSERT_TRUE(dg3 != nullptr);
    for (size_t cg_index = 0; cg_index < 2; ++cg_index) {
      auto* cg3 = writer->CreateChannelGroup(dg3);
      ASSERT_TRUE(cg3 != nullptr);
      cg3->Description("CG description");
      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* cn3 = writer->CreateChannel(cg3);
        ASSERT_TRUE(cn3 != nullptr);
        std::ostringstream name;
        name << "Channel" << cn_index;
        cn3->Name(name.str());
        cn3->Description("Channel description");
        cn3->Type(cn_index == 0 ? ChannelType::Master
                                : ChannelType::FixedLength);
        cn3->DataType(ChannelDataType::FloatBe);
        cn3->DataBytes(4);
        cn3->Unit("s");
      }
    }
  }
  writer->InitMeasurement();
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite, Mdf3WriteTest1) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("test1.mf3");
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");
  header->Department("Home Alone");
  header->Description("Testing Sample");
  header->Project("Mdf3WriteHD");
  header->StartTime(TimeStampToNs());
  header->Subject("PXY");

  auto* dg3 = writer->CreateDataGroup();
  auto* cg3 = writer->CreateChannelGroup(dg3);
  for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
    auto* cn3 = writer->CreateChannel(cg3);
    ASSERT_TRUE(cn3 != nullptr);
    std::ostringstream name;
    name << "Channel_" << cn_index + 1;
    cn3->Name(name.str());
    cn3->Description("Channel description");
    cn3->Type(cn_index == 0 ? ChannelType::Master : ChannelType::FixedLength);
    cn3->DataType(ChannelDataType::FloatBe);
    cn3->DataBytes(4);
    cn3->Unit("s");
  }

  writer->InitMeasurement();

  writer->StartMeasurement(TimeStampToNs());
  for (size_t sample = 0; sample < 100; ++sample) {
    auto cn_list = cg3->Channels();
    double value = 0.01 * static_cast<double>(sample);
    std::ranges::for_each(
        cn_list, [&](auto* channel) { channel->SetChannelValue(value, true); });
    writer->SaveSample(*cg3, TimeStampToNs());
    sleep_for(10ms);
  }
  writer->StopMeasurement(TimeStampToNs());
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite, Mdf3WriteTestValueType) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("test_types.mf3");
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);
  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");
  header->Department("Home Alone");
  header->Description("Testing Value Types");
  header->Project("Mdf3WriteHD");
  header->StartTime(TimeStampToNs());
  header->Subject("PXY");

  auto* dg3 = writer->CreateDataGroup();
  auto* cg3 = writer->CreateChannelGroup(dg3);
  {
    auto* master = writer->CreateChannel(cg3);
    master->Name("Time");
    master->Description("Time channel");
    master->Type(ChannelType::Master);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(4);
    master->Unit("s");
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("UnsignedLe");
    channel->Description("uint32_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::UnsignedIntegerLe);
    channel->DataBytes(sizeof(uint32_t));
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("UnsignedBe");
    channel->Description("uint16_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::UnsignedIntegerBe);
    channel->DataBytes(sizeof(uint16_t));
  }

  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("SignedLe");
    channel->Description("int32_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::SignedIntegerLe);
    channel->DataBytes(sizeof(int32_t));
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("SignedBe");
    channel->Description("int8_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::SignedIntegerLe);
    channel->DataBytes(sizeof(int8_t));
  }

  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("FloatLe");
    channel->Description("float");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(sizeof(float));
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("FloatBe");
    channel->Description("double");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::FloatBe);
    channel->DataBytes(sizeof(double));
  }

  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("String");
    channel->Description("string");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::StringAscii);
    channel->DataBytes(10);
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("Array");
    channel->Description("vector");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::ByteArray);
    channel->DataBytes(5);
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("Date");
    channel->Description("CANopen Date");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::CanOpenDate);
    // channel->DataBytes(7);
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("Time");
    channel->Description("CANopen Time");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::CanOpenTime);
    // channel->DataBytes(7);
  }

  writer->InitMeasurement();
  writer->StartMeasurement(TimeStampToNs());
  for (size_t sample = 0; sample < 100; ++sample) {
    auto cn_list = cg3->Channels();
    cn_list[0]->SetChannelValue(0.01 * static_cast<double>(sample));
    cn_list[1]->SetChannelValue(sample);
    cn_list[2]->SetChannelValue(sample);
    cn_list[3]->SetChannelValue(-sample);
    cn_list[4]->SetChannelValue(-sample);
    cn_list[5]->SetChannelValue(11.1 * static_cast<double>(sample));
    cn_list[6]->SetChannelValue(11.1 * static_cast<double>(sample));

    cn_list[7]->SetChannelValue(std::to_string(sample));
    std::vector<uint8_t> temp(5, 0);
    temp[0] = 'T';
    cn_list[8]->SetChannelValue(temp);

    auto ns70 = TimeStampToNs();
    cn_list[9]->SetChannelValue(ns70);
    cn_list[10]->SetChannelValue(ns70);

    writer->SaveSample(*cg3, ns70);
    sleep_for(10ms);
  }
  writer->StopMeasurement(TimeStampToNs());
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite, Mdf4WriteHD) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("hd4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));

  const auto start_time1 = TimeStampToNs();

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  EXPECT_EQ(header->Index(), 0);

  header->Author("Ingemar Hedvall");
  EXPECT_TRUE(header->Author() == "Ingemar Hedvall");

  header->Department("Home Alone");
  header->Description("Testing Header");
  header->Project("Mdf4WriteHD");
  header->StartTime(start_time1);
  header->Subject("PXY");
  header->MeasurementId("AAA");
  header->RecorderId("BBB");
  header->RecorderIndex(45);
  header->StartAngle(1.0);
  header->StartDistance(2.0);

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadHeader());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  EXPECT_EQ(header->Author(), header1->Author());
  std::cout << "Author: " << header1->Author() << std::endl;
  EXPECT_EQ(header->Department(), header1->Department());
  EXPECT_EQ(header->Description(), header1->Description());

  EXPECT_EQ(header->Project(), header1->Project());

  EXPECT_EQ(header->StartTime(), header1->StartTime());
  EXPECT_EQ(header1->StartTime(), start_time1);

  EXPECT_EQ(header->Subject(), header1->Subject());
  EXPECT_EQ(header->MeasurementId(), header1->MeasurementId());
  EXPECT_EQ(header->RecorderId(), header1->RecorderId());
  EXPECT_EQ(header->RecorderIndex(), header1->RecorderIndex());

  EXPECT_EQ(header->StartAngle(), header1->StartAngle());
  EXPECT_TRUE(header1->StartAngle().has_value());

  EXPECT_EQ(header->StartDistance(), header1->StartDistance());
  EXPECT_TRUE(header1->StartDistance().has_value());
}

TEST_F(TestWrite, Mdf4WriteFH) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("fd4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");

  auto time_stamp = TimeStampToNs();
  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  EXPECT_EQ(history->Index(), 0);
  history->Time(time_stamp);
  history->Description("Initial stuff");
  history->ToolName("Unit Test");
  history->ToolVendor("ACME");
  history->ToolVersion("2.3");
  history->UserName("Ducky");

  EXPECT_FALSE(history->Description().empty());

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadHeader());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto fh_list = header1->FileHistories();
  ASSERT_EQ(fh_list.size(), 1);

  const auto* history1 = fh_list[0];
  ASSERT_TRUE(history1 != nullptr);
  EXPECT_EQ(history->Time(), history1->Time());
  EXPECT_EQ(history->Description(), history1->Description());
  EXPECT_EQ(history->ToolName(), history1->ToolName());
  EXPECT_EQ(history->ToolVendor(), history1->ToolVendor());
  EXPECT_EQ(history->ToolVersion(), history1->ToolVersion());
  EXPECT_EQ(history->UserName(), history1->UserName());
}

TEST_F(TestWrite, Mdf4WriteAT) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("at4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  const auto& log_config = LogConfig::Instance();
  const auto log_file = log_config.GetLogFile();

  for (size_t count = 0; count < 3; ++count) {
    auto* attachment = header->CreateAttachment();
    ASSERT_TRUE(attachment != nullptr);
    EXPECT_EQ(attachment->Index(), 0);
    attachment->CreatorIndex(count);
    EXPECT_EQ(attachment->CreatorIndex(), count);

    attachment->IsEmbedded(count > 0);
    attachment->IsCompressed(count > 1);
    attachment->FileName(log_file);
    attachment->FileType("text/plain");
    const auto md5 = attachment->Md5();
    EXPECT_FALSE(md5.has_value());
  }

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadMeasurementInfo());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto at_list = header1->Attachments();
  ASSERT_EQ(at_list.size(), 3);

  for (size_t count1 = 0; count1 < at_list.size(); ++count1) {
    const auto* attachment = at_list[count1];
    ASSERT_TRUE(attachment != nullptr);
    EXPECT_EQ(attachment->CreatorIndex(), count1);
    EXPECT_EQ(attachment->IsEmbedded(), count1 > 0);
    EXPECT_EQ(attachment->IsCompressed(), count1 > 1);
    EXPECT_EQ(attachment->FileName(), log_file);
    EXPECT_STREQ(attachment->FileType().c_str(), "text/plain");
    EXPECT_TRUE(attachment->Md5().has_value());
    EXPECT_EQ(attachment->Md5().value().size(), 32);
  }
}

TEST_F(TestWrite, Mdf4WriteEV) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("ev4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Created");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* event = header->CreateEvent();
  ASSERT_TRUE(event != nullptr);
  EXPECT_EQ(event->Index(), 0);
  event->GroupName("Olle");
  event->Description("Olle Event");
  EXPECT_STREQ(event->Description().c_str(), "Olle Event");

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto ev_list = header1->Events();
  ASSERT_EQ(ev_list.size(), 1);
}

TEST_F(TestWrite, Mdf4WriteDG) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("dg4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Created");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  ASSERT_TRUE(data_group != nullptr);
  EXPECT_EQ(data_group->Index(), 0);
  data_group->Description("Olle Meas");
  EXPECT_STREQ(data_group->Description().c_str(), "Olle Meas");

  auto* channel_group = data_group->CreateChannelGroup();
  ASSERT_TRUE(channel_group != nullptr);
  EXPECT_EQ(channel_group->Index(), 0);

  channel_group->Name("Groupie");
  EXPECT_STREQ(channel_group->Name().c_str(), "Groupie");
  channel_group->Description("Groupie desc");
  EXPECT_STREQ(channel_group->Description().c_str(), "Groupie desc");

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto dg_list = header1->DataGroups();
  ASSERT_EQ(dg_list.size(), 1);

  const auto cg_list = dg_list[0]->ChannelGroups();
  ASSERT_EQ(cg_list.size(), 1);
}

}  // end namespace mdf::test