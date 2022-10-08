/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "ri4block.h"
namespace mdf::detail {

size_t Ri4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  data_position_ = GetFilePosition(file);
  return bytes;
}

size_t Ri4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}

} // namespace mdf::detail