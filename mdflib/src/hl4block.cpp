/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "hl4block.h"
namespace {

constexpr size_t kIndexNext = 0; ///< First DL block

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Equal";
  }
  return s.str();
}
std::string MakeZipTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "ZLIB Deflate";
    case 1:
      return "Transposition + ZLIB Deflate";
    default:
      break;
  }
  return "Unknown";
}
}  // namespace

namespace mdf::detail {
void Hl4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("First DL", ToHexString(Link(kIndexNext)),
                    "Link to next attach", BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

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
  ReadBlockList(file, kIndexNext);
  return bytes;
}

size_t Hl4Block::Write(std::FILE *file) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  if (update) {
    // Note that the block_list is used for the dl_list
    WriteLink4List(file, block_list_,kIndexNext,
               UpdateOption::DoNotUpdateWrittenBlock); // Only save non-updated
    return block_length_;
  }

  block_type_ = "##HL";
  block_length_ = 24 + (1*8) + 2;
  link_list_.resize(1, 0);

  auto bytes = MdfBlock::Write(file);
  bytes += WriteNumber(file,flags_);

  UpdateBlockSize(file, bytes);

  WriteLink4List(file, block_list_,kIndexNext,
            UpdateOption::DoNotUpdateWrittenBlock); // Only save non-updated
  return bytes;
}
}  // namespace mdf::detail