/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dl4block.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexData = 1;

std::string MakeFlagString(uint8_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Equal";
  }
  return s.str();
}
}
namespace mdf::detail {
void Dl4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next DL", ToHexString(Link(kIndexNext)), "Link to next attach", BlockItemType::LinkItem );
  for (size_t ii = 0; ii < nof_blocks_; ++ii) {
    dest.emplace_back("Data", ToHexString(Link(kIndexData + ii)), "Link to data block", BlockItemType::LinkItem );
  }
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);

  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Nof Data Blocks", std::to_string(nof_blocks_));
  dest.emplace_back("Equal Length", std::to_string(equal_length_));
  for (auto offset : offset_list_) {
    dest.emplace_back("Offset", std::to_string(offset), "Offset in data block");
  }
}

size_t Dl4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 3);
  bytes += ReadNumber(file, nof_blocks_);
  if (flags_ & Dl4Flags::EqualLength) {
    bytes += ReadNumber(file, equal_length_);
  } else {
    for (uint32_t ii = 0; ii < nof_blocks_; ++ii) {
      uint64_t offset = 0;
      bytes += ReadNumber(file, offset);
      offset_list_.push_back(offset);
    }
  }
  ReadLinkList(file, kIndexData, nof_blocks_);
  return bytes;
}
}