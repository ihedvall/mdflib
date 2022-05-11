/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "sd4block.h"

namespace mdf::detail {
size_t Sd4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  data_position_ = GetFilePosition(file);
  return bytes;
}

size_t Sd4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}

}