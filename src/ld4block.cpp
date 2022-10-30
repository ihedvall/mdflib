/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ld4block.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexData = 1;

std::string MakeFlagString(uint32_t flag) {
  std::ostringstream temp;
  if (flag & mdf::detail::Ld4Flags::EqualSampleCount) {
    temp << (temp.str().empty() ? "Equal" : ",Equal");
  }

  if (flag & mdf::detail::Ld4Flags::TimeValues) {
    temp << (temp.str().empty() ? "Time" : ",Time");
  }

  if (flag & mdf::detail::Ld4Flags::AngleValues) {
    temp << (temp.str().empty() ? "Angle" : ",Angle");
  }

  if (flag & mdf::detail::Ld4Flags::DistanceValues) {
    temp << (temp.str().empty() ? "Distance" : ",Distance");
  }

  if (flag & mdf::detail::Ld4Flags::InvalidData) {
    temp << (temp.str().empty() ? "Invalid" : ",Invalid");
  }
  return temp.str();
}

}  // namespace

namespace mdf::detail {
void Ld4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next LD", ToHexString(Link(kIndexNext)),
                    "Link to next attach", BlockItemType::LinkItem);
  for (size_t ii = 0; ii < nof_blocks_; ++ii) {
    dest.emplace_back("Data Block", ToHexString(Link(kIndexData + ii)),
                      "Link to data block", BlockItemType::LinkItem);
  }
  if (flags_ & Ld4Flags::InvalidData) {
    for (size_t ii = 0; ii < nof_blocks_; ++ii) {
      dest.emplace_back("Invalid Block",
                        ToHexString(Link(kIndexData + nof_blocks_ + ii)),
                        "Link to invalid block", BlockItemType::LinkItem);
    }
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);

  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Nof Data Blocks", std::to_string(nof_blocks_));
  dest.emplace_back("Equal Sample Count", std::to_string(equal_sample_count_));
  for (auto offset : offset_list_) {
    dest.emplace_back("Offset", std::to_string(offset), "Offset in data block");
  }
}

size_t Ld4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, nof_blocks_);

  if (flags_ & Ld4Flags::EqualSampleCount) {
    bytes += ReadNumber(file, equal_sample_count_);
  } else {
    for (uint32_t ii = 0; ii < nof_blocks_; ++ii) {
      uint64_t offset = 0;
      bytes += ReadNumber(file, offset);
      offset_list_.push_back(offset);
    }
  }
  if (flags_ & Ld4Flags::TimeValues) {
    for (uint32_t ii = 0; ii < nof_blocks_; ++ii) {
      int64_t value = 0;
      bytes += ReadNumber(file, value);
      time_values_.push_back(value);
    }
  }
  if (flags_ & Ld4Flags::AngleValues) {
    for (uint32_t ii = 0; ii < nof_blocks_; ++ii) {
      int64_t value = 0;
      bytes += ReadNumber(file, value);
      angle_values_.push_back(value);
    }
  }
  if (flags_ & Ld4Flags::DistanceValues) {
    for (uint32_t ii = 0; ii < nof_blocks_; ++ii) {
      int64_t value = 0;
      bytes += ReadNumber(file, value);
      distance_values_.push_back(value);
    }
  }
  ReadLinkList(file, kIndexData, nof_blocks_);
  return bytes;
}
}  // namespace mdf::detail
