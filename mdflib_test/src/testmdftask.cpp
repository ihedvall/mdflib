/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "testmdftask.h"

#include <string>
#include <string_view>
#include <map>
#include <filesystem>



#include "mdf/mdftask.h"
#include "mdf/mdflogstream.h"

using namespace std::filesystem;
using MdfList = std::map<std::string, std::string>;

namespace {
constexpr std::string_view kMdfDir = "k:/test/mdf";  // Where all source MDF files exist


MdfList kMdfList; // List of all MDF files

std::string kTestDir; ///< Temp Dir/test/mdf/task";
bool kSkipTest = false;

std::string GetMdfFile(const std::string &name) {
  auto itr = kMdfList.find(name);
  return itr == kMdfList.cend() ? std::string() : itr->second;
}

}

namespace mdf::test {

void TestMdfTask::SetUpTestSuite() {

  try {
    MdfLogStream::SetLogFunction1(MdfLogStream::LogToConsole);

    path temp_dir = temp_directory_path();
    temp_dir.append("test");
    temp_dir.append("mdf");
    temp_dir.append("task");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      MDF_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

    MDF_TRACE() << "Created the test directory. Dir: " << kTestDir;

    kMdfList.clear();

    for (const auto &entry : recursive_directory_iterator(kMdfDir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &fullname = entry.path();
      if (const auto stem = fullname.stem().string();
        kMdfList.find(stem) == kMdfList.cend() && IsMdfFile(fullname.string())) {
        kMdfList.emplace(stem, fullname.string());
      }
    }

    MDF_INFO() << "Found " << kMdfList.size() << " MDF files";
    kSkipTest = kMdfList.empty();

  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestMdfTask::TearDownTestSuite() {
  MDF_TRACE() << "Tear down the test suite.";
  MdfLogStream::ResetLogFunction();
}

TEST_F(TestMdfTask, TestProperties) {
}

}  // namespace mdf::test