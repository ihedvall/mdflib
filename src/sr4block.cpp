/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "sr4block.h"
namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexData = 1;

std::string MakeSyncTypeString(uint8_t type) {
  switch (type) {
    case 0: return "";
    case 1: return "Timer";
    case 2: return "Angle";
    case 3: return "Distance";
    case 4: return "Index";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Invalid";
  }
  return s.str();
}

}

namespace mdf::detail {

void Sr4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next SR", ToHexString(Link(kIndexNext)), "Link to next sample reduction", BlockItemType::LinkItem );
  dest.emplace_back("Data", ToHexString(Link(kIndexData)), "Link to data",BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Samples", std::to_string(nof_samples_));
  dest.emplace_back("Length", ToString(interval_));
  dest.emplace_back("Sync Type", MakeSyncTypeString(type_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
}

size_t Sr4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, nof_samples_);
  bytes += ReadNumber(file, interval_);
  bytes += ReadNumber(file, type_);
  bytes += ReadNumber(file, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 6);
  ReadBlockList(file, kIndexData);
  return bytes;
}

}