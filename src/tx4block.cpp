/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <sstream>
#include "tx4block.h"
#include "util/stringutil.h"


namespace mdf::detail {

Tx4Block::Tx4Block(const std::string &text)
: text_(text) {
}

std::string FixCommentToLine(const std::string& comment, size_t max) {
  std::ostringstream temp;
  size_t count = 0;
  for (const char in : comment) {
    if (!std::isprint(in)) {
      break;
    }
    temp << in;
    ++count;
    if (count >= max) {
      temp << " ...";
      break;
    }
  }
  return temp.str();
}

bool Tx4Block::IsTxtBlock() const {
  return util::string::IEquals(block_type_, "##TX", 4);
}

size_t Tx4Block::Read(std::FILE *file) {
  auto bytes = ReadHeader4(file);

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

std::string Tx4Block::Text() const {
  std::string temp = text_;
  util::string::Trim(temp);
  return temp;
}

std::string Tx4Block::TxComment() const {
  return FixCommentToLine(Text(), 40);
}

void Tx4Block::GetBlockProperty(BlockPropertyList &dest) const {
  dest.emplace_back("Comment", Text());
}


}