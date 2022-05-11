/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <sstream>
#include <cstdio>
#include <util/stringutil.h>
#include "tx3block.h"

namespace mdf::detail {

Tx3Block::Tx3Block(const std::string &text)
: text_(text) {
}

size_t Tx3Block::Read(std::FILE *file) {
  auto bytes = ReadHeader3(file);

  std::ostringstream temp;
  char in = '\0';
  for (; bytes < block_size_; ++bytes) {
    auto nof = std::fread(&in, 1, 1, file);
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

std::string Tx3Block::Text() const {
  std::string temp = text_;
  util::string::Trim(temp);
  return temp;
}
size_t Tx3Block::Write(std::FILE *file) {
  block_type_ = "TX";
  block_size_ = (2 + 2) + text_.size() + 1;
  link_list_.clear();

  auto bytes = IBlock::Write(file);
  bytes += std::fwrite(text_.data(), 1, text_.size(), file);
  constexpr char blank = '\0';
  bytes += std::fwrite(&blank, 1, 1, file);
  return bytes;
}

}