/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "di4block.h"

namespace mdf::detail {

void Di4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Data Size [byte]", std::to_string(DataSize()));
}

size_t Di4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  data_position_ = GetFilePosition(file);
  return bytes;
}

size_t Di4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}

}  // namespace mdf::detail