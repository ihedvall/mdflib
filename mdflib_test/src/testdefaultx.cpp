/*
* Copyright 2026 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <string_view>

#include "mdf/mdfreader.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdffactory.h"


using namespace std::filesystem;

namespace mdf::test {

TEST(TestDefaultX, ReadReferenceFile) {
  constexpr std::string_view kTestFile =
    "k:/test/mdf/mdf4_2/ChannelInfo/DefaultX/Vector_DefaultX.mf4";

  if (!exists(kTestFile)) {
    GTEST_SKIP_("Test file does not exist");
  }
  MdfReader reader(kTestFile);
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());

  const auto* file = reader.GetFile();
  ASSERT_TRUE(file != nullptr);

  const auto* header = file->Header();
  ASSERT_TRUE(header != nullptr);

  size_t found_reference = 0;
  const IChannel* default_x_channel = nullptr;

  const auto dg_list = header->DataGroups();
  EXPECT_EQ(dg_list.size(), 2);
  for (const auto* data_group : dg_list) {
    ASSERT_TRUE(data_group != nullptr);
    const auto cg_list = data_group->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (const auto* channel_group : cg_list) {
      ASSERT_TRUE(channel_group != nullptr);
      const auto cn_list = channel_group->Channels();
      EXPECT_EQ(cn_list.size(), 4);
      for (const auto* channel : cn_list) {
        ASSERT_TRUE(channel != nullptr);
        if (channel->Name() == "Default X for Ax") {
          default_x_channel = channel;
          std::cout << "Found Default X: " << channel->Name() << std::endl;
        }
        const ElementLink default_x = channel->DefaultX();
        if (!default_x.IsEmpty()) {
          ++found_reference;
          EXPECT_EQ(default_x.channel, default_x_channel);
          EXPECT_EQ(default_x_channel, channel_group->GetXChannel(*channel));
          std::cout << "Found Reference: " << channel->Name() << std::endl;
        }
      }
    }
  }
  EXPECT_EQ(found_reference, 2);
  EXPECT_TRUE(default_x_channel != nullptr);
}

TEST(TestDefaultX, WriteDefaultX) {
  path temp_dir = temp_directory_path();
  temp_dir.append("test");
  temp_dir.append("mdf");
  temp_dir.append("default_x");
  std::error_code err;
  remove_all(temp_dir, err);
  create_directories(temp_dir);

  path test_file(temp_dir);
  test_file.append("test_default_x.mf4");

  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer);
    ASSERT_TRUE(writer->Init(test_file.string()));

    const auto start_time = MdfHelper::NowNs();

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);

    header->Author("Ingemar Hedvall");
    EXPECT_TRUE(header->Author() == "Ingemar Hedvall");

    header->Department("IH Development");
    header->Description("Testing DefaultX");
    header->Project("Mdf4WriteDefaultX");
    header->StartTime(start_time);
    header->Subject("PXY");

    auto* data_group = header->CreateDataGroup();
    ASSERT_TRUE(data_group != nullptr);

    auto* channel_group = data_group->CreateChannelGroup();
    ASSERT_TRUE(channel_group != nullptr);
    channel_group->Name("DefaultX");

    auto* master = channel_group->CreateChannel();
    ASSERT_TRUE(master != nullptr);
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(4);
    master->Unit("s");

    auto* channel = channel_group->CreateChannel();
    ASSERT_TRUE(channel != nullptr);
    channel->Name("Default X Reference");
    channel->Type(ChannelType::FixedLength);
    channel->Sync(ChannelSyncType::None);
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(4);
    ElementLink default_x;
    default_x.data_group = data_group;
    default_x.channel_group = channel_group;
    default_x.channel = master;
    channel->DefaultX(default_x);

    writer->InitMeasurement();
    writer->FinalizeMeasurement();

  }

  if (!exists(test_file)) {
    FAIL();
  }

  {

    MdfReader reader(test_file.string());
    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadEverythingButData());

    const auto* file = reader.GetFile();
    ASSERT_TRUE(file != nullptr);

    const auto* header = file->Header();
    ASSERT_TRUE(header != nullptr);

    size_t found_reference = 0;
    const IChannel* default_x_channel = nullptr;

    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 1);
    for (const auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);
      const auto cg_list = data_group->ChannelGroups();
      EXPECT_EQ(cg_list.size(), 1);
      for (const auto* channel_group : cg_list) {
        ASSERT_TRUE(channel_group != nullptr);
        const auto cn_list = channel_group->Channels();
        EXPECT_EQ(cn_list.size(), 2);
        for (const auto* channel : cn_list) {
          ASSERT_TRUE(channel != nullptr);
          if (channel->Name() == "Time") {
            default_x_channel = channel;
            std::cout << "Found Default X: " << channel->Name() << std::endl;
          }
          const ElementLink default_x = channel->DefaultX();
          if (!default_x.IsEmpty()) {
            ++found_reference;
            EXPECT_EQ(default_x.channel, default_x_channel);
            EXPECT_EQ(default_x_channel, channel_group->GetXChannel(*channel));
            std::cout << "Found Reference: " << channel->Name() << std::endl;
          }
        }
      }
    }
    EXPECT_EQ(found_reference, 1);
    EXPECT_TRUE(default_x_channel != nullptr);
  }
}
}