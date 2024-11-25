/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include "mdfblock.h"
#include <filesystem>
#include <fstream>
using namespace std::filesystem;
using namespace mdf::detail;
namespace {
  std::string MakeTestFilePath(const std::string_view& name) {
    try {
      path filename = temp_directory_path();
      filename.append("test");
      filename.append("mdf");
      create_directories(filename);
      filename.append(name);
      remove(filename);
      return filename.string();

    } catch (const std::exception& err) {
     std::cout << err.what() << std::endl;
    }
    return {};
  }

}

namespace mdf::test {

TEST(TestMdfBlock, TestStatic) {
  std::string test_file = MakeTestFilePath("static.mf4");
  std::filebuf file_buffer;
  std::filebuf* write = file_buffer.open(test_file,
                              std::ios_base::out
                              | std::ios_base::trunc
                              | std::ios_base::binary );
  ASSERT_TRUE(write != nullptr);
  EXPECT_TRUE(file_buffer.is_open());

  EXPECT_EQ(GetFilePosition(file_buffer), 0);
  EXPECT_EQ(GetLastFilePosition(file_buffer), 0);

  std::vector<uint8_t> orig_array = {0,1,2,3,4,5,6,7,8,9};
  const uint64_t orig_size = WriteByte(file_buffer, orig_array);
  EXPECT_EQ(orig_size, 10);

  const uint64_t empty_size = WriteBytes(file_buffer, 10);
  EXPECT_EQ(empty_size, 10);

  const uint64_t text_size = WriteStr(file_buffer, "Hello", 10);
  EXPECT_EQ(text_size, 10);

  file_buffer.close();
  EXPECT_FALSE(file_buffer.is_open());

  std::filebuf* read = file_buffer.open(test_file,
                 std::ios_base::in | std::ios_base::binary);
  ASSERT_TRUE(read != nullptr);
  EXPECT_TRUE(file_buffer.is_open());

  EXPECT_EQ(GetFilePosition(file_buffer), 0);
  EXPECT_GT(GetLastFilePosition(file_buffer), 0);
  SetFirstFilePosition(file_buffer);

  std::vector<uint8_t> dest_array;
  const uint64_t dest_size = ReadByte(file_buffer, dest_array, 10);
  EXPECT_EQ(dest_array, orig_array);
  EXPECT_EQ(dest_size, 10);

  const uint64_t step_size = StepFilePosition(file_buffer, 10);
  EXPECT_EQ(step_size, 10);

  std::string dest_text;
  const uint64_t read_size = ReadStr(file_buffer, dest_text, 10);
  EXPECT_STREQ(dest_text.c_str(), "Hello");
  EXPECT_EQ(read_size, 10 );

  file_buffer.close();
  EXPECT_FALSE(file_buffer.is_open());

}

TEST(TestMdfBlock, TestOpenMdf) {
  std::string test_file = MakeTestFilePath("open.mf4");
  std::filebuf file_buffer;
  const bool write = OpenMdfFile(file_buffer, test_file,
                                         std::ios_base::out
                                             | std::ios_base::trunc
                                             | std::ios_base::binary );
  ASSERT_TRUE(write);
  EXPECT_TRUE(file_buffer.is_open());
  std::vector<uint8_t> orig_array = {0,1,2,3,4,5,6,7,8,9};
  WriteByte(file_buffer, orig_array);

  file_buffer.close();
  EXPECT_FALSE(file_buffer.is_open());

  const bool read = OpenMdfFile(file_buffer, test_file,
                                        std::ios_base::in | std::ios_base::binary);
  ASSERT_TRUE(read);
  EXPECT_TRUE(file_buffer.is_open());

  std::vector<uint8_t> dest_array;
  ReadByte(file_buffer, dest_array, 10);
  EXPECT_EQ(dest_array, orig_array);

  file_buffer.close();
  EXPECT_FALSE(file_buffer.is_open());

}

}
