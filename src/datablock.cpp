/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <array>
#include "datablock.h"
namespace mdf::detail
{

size_t DataBlock::CopyDataToFile(std::FILE *from_file, std::FILE *to_file) const {
  if (from_file == nullptr || to_file == nullptr) {
    throw std::invalid_argument("File pointers may not be null");
  }

  SetFilePosition(from_file, DataPosition());

  auto data_size = DataSize();
  if (data_size == 0) {
    return 0;
  }
  size_t count = 0;
  std::array<uint8_t, 10'000> temp {};
  size_t bytes_to_read = std::min(data_size, temp.size());
  for (auto reads = fread(temp.data(),1,bytes_to_read,from_file);
       reads > 0 && bytes_to_read > 0 && data_size > 0;
       reads = fread(temp.data(),1, bytes_to_read, from_file)) {
      const auto writes = fwrite(temp.data(), 1, reads, to_file);
      count += writes;
      if (writes != reads) {
        break;
      }

    data_size-= reads;
    bytes_to_read = std::min(data_size, temp.size());
  }
  return count;
}

size_t DataBlock::CopyDataToBuffer(std::FILE *from_file, std::vector<uint8_t> &buffer, size_t& buffer_index) const {
  if (from_file == nullptr) {
    throw std::invalid_argument("File pointers may not be null");
  }
  SetFilePosition(from_file, DataPosition());
  const auto data_size = DataSize();
  if (data_size == 0) {
    return 0;
  }
  const auto reads = fread(buffer.data() + buffer_index,1,data_size,from_file);
  buffer_index += reads;
  return reads;
}

}