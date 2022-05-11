/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "si4block.h"
namespace {

constexpr size_t kIndexName = 0;
constexpr size_t kIndexPath = 1;
constexpr size_t kIndexMd = 2;

std::string MakeSourceTypeString(uint8_t type) {
  switch (type) {
    case 0: return "Other";
    case 1: return "ECU";
    case 2: return "Bus";
    case 3: return "I/O";
    case 4: return "Tool";
    case 5: return "User";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeBusTypeString(uint8_t type) {
  switch (type) {
    case 0: return "";
    case 1: return "Other";
    case 2: return "CAN";
    case 3: return "LIN";
    case 4: return "MOST";
    case 5: return "FlexRay";
    case 6: return "K-Line";
    case 7: return "Ethernet";
    case 8: return "USB";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Simulated";
  }
  return s.str();
}

}
namespace mdf::detail {

void Si4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), "Link to name text", BlockItemType::LinkItem );
  dest.emplace_back("Path TX", ToHexString(Link(kIndexPath)), "Link to path text",BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name", name_);
  dest.emplace_back("Path", path_);
  dest.emplace_back("Source Type", MakeSourceTypeString(type_));
  dest.emplace_back("Bus Type", MakeBusTypeString(type_));
  dest.emplace_back("Flags", MakeFlagString(flags_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}
size_t Si4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, type_);
  bytes += ReadNumber(file, bus_type_);
  bytes += ReadNumber(file, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 5);
  name_ = ReadTx4(file,kIndexName);
  path_ = ReadTx4(file,kIndexPath);
  ReadMdComment(file,kIndexMd);
  return bytes;
}

}