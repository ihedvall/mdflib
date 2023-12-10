/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "tx4block.h"

#include <sstream>

#include "mdf/mdfhelper.h"
#include "platform.h"

namespace mdf::detail {

Tx4Block::Tx4Block(const std::string &text) : text_(text) {}

std::string FixCommentToLine(const std::string &comment, size_t max) {
  std::ostringstream temp;
  size_t count = 0;
  for (auto input : comment) {
    const auto in = static_cast<unsigned char>(input);
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
  return Platform::strnicmp(block_type_.c_str(), "##TX", 4) == 0;
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

size_t Tx4Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  const bool is_xml = !text_.empty() && text_[0] == '<';
  block_type_ = is_xml ? "##MD" : "##TX";
  block_size_ = static_cast<uint16_t>(24 + text_.size() + 1);
  link_list_.clear();

  auto bytes = MdfBlock::Write(file);
  bytes += WriteStr(file, text_, text_.size());
  bytes += WriteBytes(file, 1);
  UpdateBlockSize(file, bytes);
  return bytes;
}

std::string Tx4Block::Text() const {
  std::string temp = text_;
  MdfHelper::Trim(temp);
  return temp;
}

std::string Tx4Block::TxComment() const { return FixCommentToLine(Text(), 40); }

void Tx4Block::GetBlockProperty(BlockPropertyList &dest) const {
  dest.emplace_back("Comment", Text());
}

}  // namespace mdf::detail