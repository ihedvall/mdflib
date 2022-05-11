/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <sstream>
#include <cstdio>
#include "pr3block.h"
namespace mdf::detail {

Pr3Block::Pr3Block(const std::string &meta_data)
: text_(meta_data) {
}

size_t Pr3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);

  std::ostringstream temp;
  char in = '\0';
  for (; bytes < block_size_; ++bytes) {
    size_t nof = std::fread(&in, 1, 1, file);
    if (nof != 1) {
      // Suppose end of file or something
      break;
    }
    if (in == '\0') {
      // No more to read
      break;
    }
    temp << in;
  }
  text_ = temp.str();

  return bytes;
}

size_t Pr3Block::Write(std::FILE *file) {
  block_type_ = "PR";
  block_size_ = (2 + 2) + text_.size();
  link_list_.clear();

  auto bytes = IBlock::Write(file);
  if (!text_.empty()) {
    bytes += std::fwrite(text_.data(), 1, text_.size(), file);
  }
  return bytes;
}

}
