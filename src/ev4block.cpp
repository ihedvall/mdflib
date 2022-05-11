/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ev4block.h"

namespace {
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexParent = 1;
constexpr size_t kIndexRange = 2;
constexpr size_t kIndexName = 3;
constexpr size_t kIndexMd = 4;
constexpr size_t kIndexScope = 5;

std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0: return "Recording";
    case 1: return "Recording Interrupt";
    case 2: return "Acquisition Interrupt";
    case 3: return "Start Recording";
    case 4: return "Stop Recording";
    case 5: return "Trigger";
    case 6: return "Marker";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeSyncTypeString(uint8_t type) {
  switch (type) {
    case 0: return "";
    case 1: return "Time Calculated";
    case 2: return "Angle Calculated";
    case 3: return "Distance Calculated";
    case 4: return "Index Calculated";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeRangeTypeString(uint8_t type) {
  switch (type) {
    case 0: return "Point";
    case 1: return "Begin";
    case 2: return "End";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeCauseString(uint8_t cause) {
  switch (cause) {
    case 0: return "Other";
    case 1: return "Error";
    case 2: return "Tool";
    case 3: return "Script";
    case 4: return "User";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint8_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Post";
  }
  return s.str();
}
}

namespace mdf::detail {
void Ev4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next EV", ToHexString(Link(kIndexNext)), "Link to next event", BlockItemType::LinkItem );
  dest.emplace_back("Parent EV", ToHexString(Link(kIndexParent)), "Reference to parent event",BlockItemType::LinkItem );
  dest.emplace_back("Range EV", ToHexString(Link(kIndexRange)), "Reference to range begin event", BlockItemType::LinkItem );
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), "Link to name", BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  for (size_t m = 0; m < length_m_; ++m) {
    dest.emplace_back("Scope CG/CN", ToHexString(Link(kIndexScope + m)), "Reference to scope ",BlockItemType::LinkItem );
  }
  for (size_t n = 0; n < length_n_; ++n) {
    dest.emplace_back("Attachment AT", ToHexString(Link(kIndexScope + length_m_ + n)), "Reference to attachments ",BlockItemType::LinkItem );
  }
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name", name_);
  dest.emplace_back("Event Type", MakeTypeString(type_));
  dest.emplace_back("Sync Type", MakeSyncTypeString(sync_type_));
  dest.emplace_back("Range Type", MakeRangeTypeString(range_type_));
  dest.emplace_back("Cause", MakeCauseString(cause_));
  dest.emplace_back("Flags", MakeFlagString(flags_));

  dest.emplace_back("Nof Scopes", std::to_string(length_m_));
  dest.emplace_back("Nof Attachments", std::to_string(length_n_));
  dest.emplace_back("Creator Index", std::to_string(creator_index_));
  dest.emplace_back("Sync Base Value", std::to_string(sync_base_value_));
  dest.emplace_back("Sync Factor", ToString(sync_factor_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Ev4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, type_);
  bytes += ReadNumber(file, sync_type_);
  bytes += ReadNumber(file, range_type_);
  bytes += ReadNumber(file, cause_);
  bytes += ReadNumber(file, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 3);
  bytes += ReadNumber(file, length_m_);
  bytes += ReadNumber(file, length_n_);
  bytes += ReadNumber(file, creator_index_);
  bytes += ReadNumber(file, sync_base_value_);
  bytes += ReadNumber(file, sync_factor_);
  name_ = ReadTx4(file,kIndexName);
  ReadMdComment(file,kIndexMd);
  return bytes;
}
}