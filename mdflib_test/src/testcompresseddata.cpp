/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdffactory.h"
#include "mdf/mdfhelper.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"

using namespace std::filesystem;
using namespace mdf;

namespace {

std::string CreateTestFile(const std::string& filename) {
  path fullname = temp_directory_path();
  fullname.append("test");
  fullname.append("mdf");
  fullname.append("compresseddata");
  create_directories(fullname);
  fullname.append(filename);
  remove(fullname);
  return fullname.string();
}

}  // namespace

namespace mdf::test {

// With TransposeData(), a single channel group without record ID is stored in
// transposed DZ blocks. The samples need more than one 4 MB block.
TEST(CompressedData, TransposedBlocks) {
  constexpr size_t nof_samples = 300'000;
  constexpr uint32_t record_size = 8 + 8;
  const MdfWriterType types[] = {MdfWriterType::Mdf4Basic,
                                 MdfWriterType::MdfConverter};
  for (const auto type : types) {
    const auto filename = CreateTestFile(
        type == MdfWriterType::Mdf4Basic ? "basic.mf4" : "converter.mf4");
    {
      auto writer = MdfFactory::CreateMdfWriter(type);
      ASSERT_TRUE(writer->Init(filename));
      writer->CompressData(true);
      writer->TransposeData(true);
      auto* dg = writer->CreateDataGroup();
      auto* cg = MdfWriter::CreateChannelGroup(dg);
      ASSERT_TRUE(cg != nullptr);
      auto* master = MdfWriter::CreateChannel(cg);
      master->Name("Time");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      auto* counter = MdfWriter::CreateChannel(cg);
      counter->Name("Counter");
      counter->DataType(ChannelDataType::SignedIntegerLe);
      counter->DataBytes(8);

      writer->PreTrigTime(0.0);
      ASSERT_TRUE(writer->InitMeasurement());
      uint64_t tick_time = MdfHelper::NowNs();
      writer->StartMeasurement(tick_time);
      for (size_t sample = 0; sample < nof_samples; ++sample) {
        counter->SetChannelValue(static_cast<int64_t>(sample));
        writer->SaveSample(*cg, tick_time);
        tick_time += 1'000'000;
      }
      writer->StopMeasurement(tick_time);
      ASSERT_TRUE(writer->FinalizeMeasurement());
    }

    // DZ header: 24 bytes block header, then org block type (2), zip type (1),
    // reserved (1) and zip parameter (4). HL header: 24 bytes block header,
    // one link (8), flags (2) and zip type (1).
    std::ifstream file(filename, std::ios::binary);
    const std::vector<char> bytes((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
    size_t nof_hl = 0;
    size_t nof_dz = 0;
    for (size_t pos = 0; pos + 40 <= bytes.size(); pos += 8) {
      if (std::memcmp(bytes.data() + pos, "##HL", 4) == 0) {
        ++nof_hl;
        EXPECT_EQ(bytes[pos + 34], 1) << filename;  // Transpose and deflate
      }
      if (std::memcmp(bytes.data() + pos, "##DZ", 4) != 0) {
        continue;
      }
      ++nof_dz;
      uint32_t parameter = 0;
      std::memcpy(&parameter, bytes.data() + pos + 28, 4);
      EXPECT_EQ(bytes[pos + 26], 1) << filename;  // Transpose and deflate
      EXPECT_EQ(parameter, record_size) << filename;
    }
    EXPECT_EQ(nof_hl, 1) << filename;
    EXPECT_GE(nof_dz, 2) << filename;

    MdfReader reader(filename);
    ASSERT_TRUE(reader.ReadEverythingButData());
    auto* dg = reader.GetDataGroup(0);
    ASSERT_TRUE(dg != nullptr);
    auto* cg = dg->ChannelGroups()[0];
    auto counter = CreateChannelObserver(*dg, *cg, *cg->GetChannel("Counter"));
    ASSERT_TRUE(reader.ReadData(*dg));
    ASSERT_EQ(counter->NofSamples(), nof_samples);
    for (size_t sample = 0; sample < nof_samples; ++sample) {
      int64_t counter_value = -1;
      ASSERT_TRUE(counter->GetChannelValue(sample, counter_value));
      ASSERT_EQ(counter_value, static_cast<int64_t>(sample)) << filename;
    }
  }
}

}  // namespace mdf::test
