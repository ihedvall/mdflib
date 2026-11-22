/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <string_view>
#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>
#include <mdf/mdffactory.h>
#include <mdf/mdfwriter.h>
#include <mdf/idatagroup.h>
#include <mdf/ichannelgroup.h>
#include <mdf/mdfhelper.h>

#include "createfilelist.h"
#include "logtoconsole.h"
#include "programargument.h"

namespace {

constexpr std::string_view kPathToMdf = "k:/test/mdf/mdf4_2/buslogging";
constexpr std::string_view kPathToCssFile =
    "K:/test/mdf/CanEdgeLog/2F6913DB/00000019/00000001-61D9D80A.MF4";
}

using namespace std::filesystem;

namespace mdf::test {
TEST(CreateFileList, TestProperties) {
  auto& arguments = ProgramArgument::Instance();
  arguments.Clear();

  LogToConsole logger;
  MDF_INFO() << "This should be shown";
  arguments.NonInteractive(true);
  MDF_INFO() << "This should NOT be shown";
  arguments.NonInteractive(false);

  CreateFileList file_list;

  const auto temp_path = temp_directory_path().string();
  EXPECT_TRUE(file_list.OutputPath().empty());
  file_list.OutputPath(temp_path);
  EXPECT_EQ(temp_path, file_list.OutputPath());
}

TEST(CreateFileList, TestSameFileName) {
  auto& arguments = ProgramArgument::Instance();
  arguments.Clear();

  LogToConsole logger;

  auto test_path = temp_directory_path();
  test_path.append("test/same_name");
  remove_all(test_path);

  // Two measurement sessions with the same file name in different folders.
  std::vector<std::string> input_list;
  for (int session = 1; session <= 2; ++session) {
    path mdf_file(test_path);
    mdf_file.append("session" + std::to_string(session));
    create_directories(mdf_file);
    mdf_file.append("00000001.mf4");

    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    writer->Init(mdf_file.string());
    auto* group = writer->Header()->CreateDataGroup()->CreateChannelGroup("Group");
    auto* signal = group->CreateChannel("Signal");
    signal->DataType(ChannelDataType::FloatLe);
    signal->DataBytes(8);

    writer->InitMeasurement();
    const auto tick_time = MdfHelper::NowNs();
    writer->StartMeasurement(tick_time);
    signal->SetChannelValue(static_cast<double>(session));
    writer->SaveSample(*group, tick_time);
    writer->StopMeasurement(tick_time);
    writer->FinalizeMeasurement();
    input_list.emplace_back(mdf_file.string());
  }

  CreateFileList creator;
  creator.OutputPath(test_path.string());
  creator.MakeFileList(input_list);
  ASSERT_EQ(creator.Files().size(), 2);
  for (const auto& file : creator.Files()) {
    creator.ConvertMdfFile(file);
  }

  // Each CSV holds the value of its own session.
  const auto value_in = [&](const char* csv_name) {
    std::ifstream csv_file(test_path / csv_name);
    std::string line;
    std::getline(csv_file, line); // Header
    std::getline(csv_file, line);
    return line;
  };
  EXPECT_EQ(value_in("00000001.csv"), "1");
  EXPECT_EQ(value_in("00000001_2.csv"), "2");
}

TEST(CreateFileList, TestBasicFunction) {
  auto& arguments = ProgramArgument::Instance();
  arguments.Clear();

  LogToConsole logger;

  // Check that the MDF directory exists, so we can run the test
  std::string test_directory;
  try {
    // Create a temporary test directory
    auto test_path = temp_directory_path();
    test_path.append("test/csv");
    test_directory = test_path.string();

    // Remove previous test run files at this point as we might
    // want to evaluate the created CSV files after the test run.
    remove_all(test_path);
    create_directories(test_path);

    path mdf_path(kPathToMdf);
    if (!exists(mdf_path)) {
      GTEST_SKIP();
    }
  } catch (const std::exception& ) {
    GTEST_SKIP();
  }

  const std::vector<std::string> input_list = {std::string(kPathToMdf)};
  CreateFileList creator;
  creator.OutputPath(test_directory);
  creator.MakeFileList(input_list);

  EXPECT_TRUE(creator.FilesOk());
  EXPECT_GT(creator.Files().size(), 0);
  EXPECT_EQ(creator.Directories().size(), 1);

  for (const auto& file : creator.Files()) {
    creator.ConvertMdfFile(file);

  }
}

TEST(CreateFileList, TestCsvLog) {
  auto& arguments = ProgramArgument::Instance();
  arguments.Clear();

  LogToConsole logger;
  std::string test_directory;

  // Check that the MDF file exists, so we can run the test
  try {
    // Create a temporary test directory
    auto test_path = temp_directory_path();
    test_path.append("test/css");
    test_directory = test_path.string();

    // Remove previous test run files at this point as we might
    // want to evaluate the created CSV files after the test run.
    remove_all(test_path);
    create_directories(test_path);

    path mdf_file(kPathToCssFile);
    if (!exists(mdf_file)) {
      GTEST_SKIP();
    }
  } catch (const std::exception& ) {
    GTEST_SKIP();
  }

  const std::vector<std::string> input_list = {std::string(kPathToCssFile)};
  CreateFileList creator;
  creator.OutputPath(test_directory);
  creator.MakeFileList(input_list);

  EXPECT_TRUE(creator.FilesOk());
  EXPECT_EQ(creator.Files().size(), 1);
  EXPECT_EQ(creator.Directories().size(), 0);

  for (const auto& file : creator.Files()) {
    creator.ConvertMdfFile(file);
  }
}

}