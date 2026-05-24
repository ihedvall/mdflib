/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "mdf/idatawriter.h"
#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"

using namespace std::filesystem;
using namespace mdf;

namespace {

std::string CreateTestFile(const std::string& filename) {
  path fullname = temp_directory_path();
  fullname.append("test");
  fullname.append("mdf");
  fullname.append("datawriter");
  create_directories(fullname);
  fullname.append(filename);
  remove(fullname);
  return fullname.string();
}

const DataWriterChannelLayout* FindLayout(
    const std::vector<DataWriterChannelLayout>& layouts,
    const std::string_view name) {
  const auto itr = std::find_if(layouts.cbegin(), layouts.cend(),
                                [&](const auto& layout) {
                                  return layout.name == name;
                                });
  return itr == layouts.cend() ? nullptr : &(*itr);
}

/**
 * Example helper that demonstrates the intended flow:
 * 1) Create channels and configure bytes/type/offset input fields.
 * 2) Create a DataWriter from the channel group.
 * 3) Fill a serialized record buffer and commit.
 * 4) Save sample using the ordinary SaveSample API.
 */
bool WriteSamplesWithDataWriterExample(const std::string& filename,
                                       size_t nof_samples = 3) {
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  if (!writer || !writer->Init(filename)) {
    return false;
  }

  auto* dg = writer->CreateDataGroup();
  auto* cg = MdfWriter::CreateChannelGroup(dg);
  if (cg == nullptr) {
    return false;
  }

  {
    auto* master = MdfWriter::CreateChannel(cg);
    if (master == nullptr) {
      return false;
    }
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(sizeof(float));
    master->Unit("s");
  }
  {
    auto* signal = MdfWriter::CreateChannel(cg);
    if (signal == nullptr) {
      return false;
    }
    signal->Name("Value");
    signal->Type(ChannelType::FixedLength);
    signal->DataType(ChannelDataType::SignedIntegerLe);
    signal->DataBytes(sizeof(int32_t));
  }

  if (!writer->InitMeasurement()) {
    return false;
  }

  const uint64_t start_time = 1'000'000'000ULL;
  writer->StartMeasurement(start_time);

  auto data_writer = cg->CreateDataWriter();
  if (!data_writer) {
    return false;
  }

  const auto layouts = data_writer->ChannelLayouts();
  const auto* time_layout = FindLayout(layouts, "Time");
  const auto* value_layout = FindLayout(layouts, "Value");
  if (time_layout == nullptr || value_layout == nullptr) {
    return false;
  }

  for (size_t sample = 0; sample < nof_samples; ++sample) {
    data_writer->Reset();
    auto& record = data_writer->Buffer();

    const float time_s = static_cast<float>(sample) * 0.001F;
    const int32_t value = static_cast<int32_t>(100 + sample);

    memcpy(record.data() + time_layout->byte_offset, &time_s, sizeof(time_s));
    memcpy(record.data() + value_layout->byte_offset, &value, sizeof(value));

    if (!data_writer->Commit()) {
      return false;
    }

    writer->SaveSample(*cg, start_time + static_cast<uint64_t>(sample) * 1'000'000ULL);
  }

  writer->StopMeasurement(start_time + 10'000'000ULL);
  return writer->FinalizeMeasurement();
}

}  // namespace

namespace mdf::test {

TEST(DataWriter, WriteAndReadBackMdf4) {
  const auto test_file = CreateTestFile("datawriter_basic.mf4");
  ASSERT_TRUE(WriteSamplesWithDataWriterExample(test_file, 3));

  MdfReader reader(test_file);
  ASSERT_TRUE(reader.ReadEverythingButData());

  auto* dg = reader.GetDataGroup(0);
  ASSERT_TRUE(dg != nullptr);
  const auto groups = dg->ChannelGroups();
  ASSERT_FALSE(groups.empty());
  auto* cg = groups[0];
  ASSERT_TRUE(cg != nullptr);

  const auto* time = cg->GetChannel("Time");
  const auto* value = cg->GetChannel("Value");
  ASSERT_TRUE(time != nullptr);
  ASSERT_TRUE(value != nullptr);

  auto time_observer = CreateChannelObserver(*dg, *cg, *time);
  auto value_observer = CreateChannelObserver(*dg, *cg, *value);
  ASSERT_TRUE(time_observer != nullptr);
  ASSERT_TRUE(value_observer != nullptr);

  reader.ReadData(*dg);

  ASSERT_EQ(time_observer->NofSamples(), 3);
  ASSERT_EQ(value_observer->NofSamples(), 3);

  for (uint64_t sample = 0; sample < 3; ++sample) {
    double time_value = 0.0;
    int64_t signal_value = 0;

    EXPECT_TRUE(time_observer->GetChannelValue(sample, time_value));
    EXPECT_TRUE(value_observer->GetChannelValue(sample, signal_value));

    // Master time is recalculated to relative time by SaveSample queue logic.
    EXPECT_NEAR(time_value, static_cast<double>(sample) * 0.001, 1E-6);
    EXPECT_EQ(signal_value, static_cast<int64_t>(100 + sample));
  }
}

TEST(DataWriter, RejectInvalidRawBufferSize) {
  const auto test_file = CreateTestFile("datawriter_invalid_size.mf4");
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer != nullptr);
  ASSERT_TRUE(writer->Init(test_file));

  auto* dg = writer->CreateDataGroup();
  auto* cg = MdfWriter::CreateChannelGroup(dg);
  ASSERT_TRUE(cg != nullptr);

  {
    auto* master = MdfWriter::CreateChannel(cg);
    ASSERT_TRUE(master != nullptr);
    master->Name("Time");
    master->Type(ChannelType::Master);
    master->Sync(ChannelSyncType::Time);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(sizeof(float));
  }
  {
    auto* signal = MdfWriter::CreateChannel(cg);
    ASSERT_TRUE(signal != nullptr);
    signal->Name("Value");
    signal->Type(ChannelType::FixedLength);
    signal->DataType(ChannelDataType::UnsignedIntegerLe);
    signal->DataBytes(sizeof(uint32_t));
  }

  ASSERT_TRUE(writer->InitMeasurement());
  auto data_writer = cg->CreateDataWriter();
  ASSERT_TRUE(data_writer != nullptr);

  std::vector<uint8_t> invalid(data_writer->RecordSize() + 1, 0);
  EXPECT_FALSE(data_writer->WriteRawRecord(invalid.data(), invalid.size()));

  std::vector<uint8_t> valid(data_writer->RecordSize(), 0);
  EXPECT_TRUE(data_writer->WriteRawRecord(valid.data(), valid.size()));
  EXPECT_TRUE(data_writer->Commit());

  // If commit succeeded we should be able to save one sample with normal flow.
  const uint64_t start_time = 2'000'000'000ULL;
  writer->StartMeasurement(start_time);
  writer->SaveSample(*cg, start_time);
  writer->StopMeasurement(start_time + 1'000'000ULL);
  EXPECT_TRUE(writer->FinalizeMeasurement());
}

}  // namespace mdf::test
