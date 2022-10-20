/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "dt4block.h"
namespace mdf::detail {

void Dt4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Data Size [byte]", std::to_string(DataSize()));
}

size_t Dt4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  data_position_ = GetFilePosition(file);
  return bytes;
}

size_t Dt4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}
void Dt4Block::UpdateDataSize(std::FILE *file) {
  int64_t last_file_position = mdf::detail::GetLastFilePosition(file);
  const auto data_size = last_file_position - data_position_;
  if (data_size > 0) {
    block_length_ = 24 + static_cast<uint64_t>(data_size);
  }
}

}  // namespace mdf::detail