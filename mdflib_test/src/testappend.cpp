/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "testappend.h"

#include <filesystem>
#include <sstream>

#include "mdf/mdfwriter.h"
#include "mdf/mdfreader.h"
#include "mdf/mdffactory.h"
#include "mdf/ifilehistory.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdflogstream.h"


using namespace std::filesystem;

namespace {
std::string kTestDir; ///< Temp Dir/test/mdf/append";
bool kSkipTest = false;

}

namespace mdf::test {

void TestAppend::SetUpTestSuite() {

  try {
    MdfLogStream::SetLogFunction1(MdfLogStream::LogToConsole);

    path temp_dir = temp_directory_path();
    temp_dir.append("test");
    temp_dir.append("mdf");
    temp_dir.append("append");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      MDF_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

    MDF_TRACE() << "Created the test directory. Dir: " << kTestDir;
    kSkipTest = false;

  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestAppend::TearDownTestSuite() {
  MDF_TRACE() << "Tear down the test suite.";
  MdfLogStream::ResetLogFunction();
}

TEST_F(TestAppend, Mdf3AppendTest) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  uint64_t sample_time = MdfHelper::NowNs();
  path mdf_file(kTestDir);
  mdf_file.append("test.mf3");

  for (size_t measurement = 0; measurement < 10; ++measurement) {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);
    ASSERT_TRUE(writer);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    if (header->Index() == 0) {
      header->Author("Ingemar Hedvall");
      header->Department("Home Alone");
      header->Description("Testing Appending Measurements");
      header->Project("Mdf3AppendTest");
      header->StartTime(sample_time);
      header->Subject("PXY");
    }

    auto* data_group = writer->CreateDataGroup();
    ASSERT_TRUE(data_group != nullptr);
    auto* channel_group = MdfWriter::CreateChannelGroup(data_group);
    ASSERT_TRUE(channel_group != nullptr);
    for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
      auto* channel = MdfWriter::CreateChannel(channel_group);
      ASSERT_TRUE(channel != nullptr);
      std::ostringstream name;
      name << "Channel_" << cn_index + 1;
      channel->Name(name.str());
      channel->Description("Channel description");
      channel->Type(cn_index == 0 ? ChannelType::Master : ChannelType::FixedLength);
      channel->DataType(ChannelDataType::FloatBe);
      channel->DataBytes(4);
      channel->Unit("s");
    }

    writer->InitMeasurement();

    writer->StartMeasurement(sample_time);
    for (size_t sample = 0; sample < 100; ++sample) {
      auto cn_list = channel_group->Channels();
      const auto value = static_cast<double>(sample);
      for(auto* channel : cn_list) {
        if (channel != nullptr && channel->Type() != ChannelType::Master)
        channel->SetChannelValue(value);
      }
      writer->SaveSample(*channel_group, sample_time);
      sample_time += 1'000'000'000; // Sample every second
    }
    writer->StopMeasurement(sample_time);
    writer->FinalizeMeasurement();
  }

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* header = reader.GetHeader();
  ASSERT_TRUE(header != nullptr);

  const auto dg_list = header->DataGroups();
  EXPECT_EQ(dg_list.size(), 10);

  for (const auto* data_group : dg_list) {
    ASSERT_TRUE(data_group != nullptr);
  }
}


TEST_F(TestAppend, Mdf4AppendTimeFromHeader) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("time_from_header.mf4");
  remove(mdf_file);
  const uint64_t first_start = MdfHelper::NowNs();
  // The second measurement starts one minute after the first one
  for (const uint64_t start : {first_start, first_start + 60'000'000'000}) {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer->Init(mdf_file.string()));
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup();
    auto* master = group->CreateChannel("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(8);
    writer->PreTrigTime(0);
    writer->InitMeasurement();
    writer->StartMeasurement(start);
    writer->SaveSample(*group, start);
    writer->SaveSample(*group, start + 1'000'000'000);
    writer->StopMeasurement(start + 1'000'000'000);
    writer->FinalizeMeasurement();
  }

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.ReadEverythingButData());
  EXPECT_EQ(reader.GetHeader()->StartTime(), first_start);
  auto* last_dg = reader.GetHeader()->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);
  auto time = CreateChannelObserver(*last_dg, "Time");
  ASSERT_TRUE(time);
  ASSERT_TRUE(reader.ReadData(*last_dg));
  double seconds = 0.0;
  EXPECT_TRUE(time->GetEngValue(0, seconds));
  EXPECT_DOUBLE_EQ(seconds, 60.0);
  EXPECT_TRUE(time->GetEngValue(1, seconds));
  EXPECT_DOUBLE_EQ(seconds, 61.0);
}

TEST_F(TestAppend, Mdf4AppendTest) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("test.mf4");
  uint64_t sample_time = MdfHelper::NowNs();

  for (size_t measurement = 0; measurement < 10; ++measurement) {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer);

    writer->Init(mdf_file.string());
    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    if (header->Index() == 0) {
      header->Author("Ingemar Hedvall");
      header->Department("Home Alone");
      header->Description("Testing Appending Measurements");
      header->Project("Mdf4AppendTest");
      header->StartTime(sample_time);
      header->Subject("PXY");
    }

    auto* history = header->CreateFileHistory();
    ASSERT_TRUE(history != nullptr);
    std::ostringstream history_name;
    history_name << "History measurement " << measurement + 1;
    history->Description("Test data types");
    history->ToolName("MdfWrite");
    history->ToolVendor("ACME Road Runner Company");
    history->ToolVersion("1.0");
    history->UserName("Ingemar Hedvall");

    auto* data_group = header->CreateDataGroup();
    ASSERT_TRUE(data_group != nullptr);

    auto* group = data_group->CreateChannelGroup();
    ASSERT_TRUE(group != nullptr);
    group->Name("Float");


    auto* master = group->CreateChannel();
    ASSERT_TRUE(master != nullptr);
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(4);

    auto* channel = group->CreateChannel();
    ASSERT_TRUE(channel != nullptr);
    channel->Name("Intel64");
    channel->Type(ChannelType::FixedLength);
    channel->Sync(ChannelSyncType::None);
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(8);

    writer->PreTrigTime(0);
    writer->InitMeasurement();

    writer->StartMeasurement(sample_time);

    for (size_t sample = 0; sample < 100; ++sample) {
      const auto value = static_cast<double>(sample);
      channel->SetChannelValue(value);
      writer->SaveSample(*group,sample_time);
      sample_time += 1'000'000'000;
    }
    writer->StopMeasurement(sample_time);
    writer->FinalizeMeasurement();
  }

  MdfReader reader(mdf_file.string());

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());

  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);

  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 10);

  for (auto* dg4 : dg_list) {
    ASSERT_TRUE(dg4 != nullptr);
    ChannelObserverList observer_list;

    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
      const auto channel_list = cg4->Channels();
      EXPECT_EQ(channel_list.size(), 2);
    }

    reader.ReadData(*dg4);

    for (auto& observer : observer_list) {
      ASSERT_TRUE(observer);
      if (observer->IsMaster()) {
        continue;
      }
      ASSERT_EQ(observer->NofSamples(), 100);
      for (size_t sample = 0; sample < 100; ++sample) {
        float channel_value = 0;
        const auto valid = observer->GetChannelValue(sample, channel_value);
        EXPECT_TRUE(valid);
        EXPECT_FLOAT_EQ(channel_value, static_cast<float>(sample))
            << observer->Name();
      }
    }
  }
  reader.Close();
}

}  // namespace test