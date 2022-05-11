/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <map>
#include <filesystem>
#include <chrono>
#include "util/logconfig.h"
#include "util/stringutil.h"
#include "util/logstream.h"
#include "mdf/mdfreader.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "testread.h"
using namespace std::filesystem;
using namespace util::string;
using namespace mdf;
using namespace mdf::detail;
using namespace util::log;
using namespace std::chrono_literals;

namespace {
const std::string mdf_source_dir = "k:/test/mdf"; // Where all source MDF files exist
const std::string log_root_dir = "o:/test";
const std::string log_file = "mdfread.log";
using MdfList = std::map<std::string, std::string, util::string::IgnoreCase>;
MdfList mdf_list;

std::string GetMdfFile(const std::string &name) {
  auto itr = mdf_list.find(name);
  return itr == mdf_list.cend() ? std::string() : itr->second;
}

}

namespace mdf::test {

void TestRead::SetUpTestSuite() {
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.RootDir(log_root_dir);
  log_config.BaseName(log_file);
  log_config.Type(util::log::LogType::LogToFile);
  log_config.CreateDefaultLogger();

  mdf_list.clear();
  try {
    for (const auto &entry: recursive_directory_iterator(mdf_source_dir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &p = entry.path();

      if (IsMdfFile(p.string())) {
        LOG_INFO() << "Found MDF file. File: " << p.stem().string();
        mdf_list.emplace(p.stem().string(), p.string());
      }
    }
  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to fetch the MDF test files. Error: " << error.what();
  }
}

void TestRead::TearDownTestSuite() {
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestRead, MdfReader) //NOLINT
{
  for (const auto &itr: mdf_list) {
    MdfReader oRead(itr.second);
    EXPECT_TRUE(oRead.IsOk()) << itr.second;
    EXPECT_TRUE(oRead.GetFile() != nullptr) << itr.second;
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << itr.second;
  }
}

TEST_F(TestRead, MdfFile) //NOLINT
{
  for (const auto &itr: mdf_list) {
    MdfReader oRead(itr.second);
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << itr.second;
    const auto *f = oRead.GetFile();
    EXPECT_TRUE(f != nullptr) << itr.second;
    if (f->IsMdf4()) {
      const auto *f4 = dynamic_cast<const Mdf4File *>(f);
      EXPECT_TRUE(f4 != nullptr);
      const auto &hd = f4->Hd();
      const auto &dg_list = hd.Dg4();
      LOG_INFO() << " File: " << itr.first << ", Nof Measurement: " << dg_list.size();
      EXPECT_FALSE(dg_list.empty()) << itr.second;
    } else {
      const auto *f3 = dynamic_cast<const Mdf3File *>(f);
      EXPECT_TRUE(f3 != nullptr);

    }
  }
}

TEST_F(TestRead, IdBlock) //NOLINT
{
  const std::string file = GetMdfFile("Vector_CustomExtensions_CNcomment");
  MdfReader oRead(file);
  EXPECT_TRUE(oRead.IsOk()) << file;

  const auto *f = dynamic_cast<const Mdf4File *>(oRead.GetFile());
  EXPECT_TRUE(f != nullptr) << file;

  const auto &id = f->Id();
  EXPECT_STREQ(id.FileId().c_str(), "MDF");
  EXPECT_STREQ(id.VersionString().c_str(), "4.10");
  EXPECT_STREQ(id.ProgramId().c_str(), "MCD11.01");
  EXPECT_EQ(id.Version(), 410);

}

TEST_F(TestRead, Benchmark) {
  {
    MdfReader oRead("K:/test/mdf/net/testfiles/test.mf4");
    const auto start = std::chrono::steady_clock::now();
    oRead.ReadMeasurementInfo();
    const auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = stop - start;
    std::cout << "Read Measurement Info TrueNas: " << diff.count() * 1000 << " ms" << std::endl;
  }
  {
    const auto start = std::chrono::steady_clock::now();
    MdfReader oRead("K:/test/mdf/net/testfiles/test.mf4");
    oRead.ReadEverythingButData();
    const auto* file = oRead.GetFile();
    DataGroupList dg_list;
    file->DataGroups(dg_list);
    for ( auto* dg : dg_list) {
      ChannelObserverList observer_list;
      auto cg_list = dg->ChannelGroups();
      for (const auto* cg : cg_list) {
        CreateChannelObserverForChannelGroup(*dg, *cg, observer_list);
      }
      oRead.ReadData(*dg);

      double eng_value = 0;
      bool valid = true;
      for (const auto& channel : observer_list) {
        size_t samples = channel->NofSamples();
        for (size_t sample = 0; sample < samples; ++sample) {
          valid = channel->GetEngValue(sample,eng_value);
        }
      }
    }

    const auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = stop - start;
    std::cout << "Everything + Conversion (TrueNas): " << diff.count() * 1000 << " ms" << std::endl;
  }

}

}
