/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"
#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "testwrite.h"

namespace {

constexpr std::string_view kLogRootDir = "o:/test";
constexpr std::string_view kLogFile = "mdf_write.log";
constexpr std::string_view kTestDir = "o:/test/mdf/write";

bool kSkipTest = false;
}

using namespace std::this_thread;
using namespace std::chrono_literals;
using namespace util::log;
using namespace util::time;
using namespace std::filesystem;
using namespace mdf;
namespace mdf::test {

void TestWrite::SetUpTestSuite() {
  auto &log_config = LogConfig::Instance();
  log_config.RootDir(kLogRootDir.data());
  log_config.BaseName(kLogFile.data());
  log_config.Type(util::log::LogType::LogToFile);
  log_config.CreateDefaultLogger();
  try {
    std::error_code err;
    remove_all(kTestDir, err);
    create_directories(kTestDir);
  } catch (const std::exception& error) {
    LOG_ERROR() << "Failed to create directories. Error: " << error.what();
    kSkipTest = true;
  }
}

void TestWrite::TearDownTestSuite() {
  LogConfig &log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestWrite,Mdf3WriteHD) {
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
    auto *dg3 = writer->CreateDataGroup();
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
        cn3->Type(cn_index == 0 ? ChannelType::Master : ChannelType::FixedLength);
        cn3->DataType(ChannelDataType::FloatBe);
        cn3->DataBytes(4);
        cn3->Unit("s");
      }
    }
  }
  writer->InitMeasurement();
  writer->FinalizeMeasurement();
}


TEST_F(TestWrite,Mdf3WriteTest1) {
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


  auto *dg3 = writer->CreateDataGroup();
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
    double value = 0.01 * sample;
    std::ranges::for_each(cn_list, [&](auto *channel) { channel->SetChannelValue(value, true); });
    writer->SaveSample(*cg3, TimeStampToNs());
    sleep_for(10ms);
  }
  writer->StopMeasurement(TimeStampToNs());
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite,Mdf3WriteTestValueType) {
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


  auto *dg3 = writer->CreateDataGroup();
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
    //channel->DataBytes(7);
  }
  {
    auto* channel = writer->CreateChannel(cg3);
    channel->Name("Time");
    channel->Description("CANopen Time");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::CanOpenTime);
    //channel->DataBytes(7);
  }


  writer->InitMeasurement();
  writer->StartMeasurement(TimeStampToNs());
  for (size_t sample = 0; sample < 100; ++sample) {
    auto cn_list = cg3->Channels();
    cn_list[0]->SetChannelValue(0.01 * sample);
    cn_list[1]->SetChannelValue(sample);
    cn_list[2]->SetChannelValue(sample);
    cn_list[3]->SetChannelValue(-sample);
    cn_list[4]->SetChannelValue(-sample);
    cn_list[5]->SetChannelValue(11.1 * sample);
    cn_list[6]->SetChannelValue(11.1 * sample);

    cn_list[7]->SetChannelValue(std::to_string(sample));
    std::vector<uint8_t> temp(5,0);
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

TEST_F(TestWrite,Mdf4WriteHD) {
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



} // end namespace mdf::test