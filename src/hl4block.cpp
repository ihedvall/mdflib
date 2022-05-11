/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "hl4block.h"
namespace {

constexpr size_t kIndexData = 0;

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Equal";
  }
  return s.str();
}
std::string MakeZipTypeString(uint8_t type) {
  switch (type) {
    case 0: return "ZLIB Deflate";
    case 1: return "Transposition + ZLIB Deflate";
    default:
      break;
  }
  return "Unknown";
}
}

namespace mdf::detail {
void Hl4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("First DL", ToHexString(Link(kIndexData)), "Link to next attach", BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);

  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Zip Type", MakeZipTypeString(type_));
}

size_t Hl4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, type_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 5);
  ReadBlockList(file, kIndexData);
  return bytes;
}
}