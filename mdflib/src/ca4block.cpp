/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ca4block.h"
namespace {
constexpr size_t kIndexArray = 0;
std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Array";
    case 1:
      return "Scaling Axis";
    case 2:
      return "Look-Up";
    case 3:
      return "Interval Axis";
    case 4:
      return "Classification Result";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeStorageString(uint8_t storage) {
  switch (storage) {
    case 0:
      return "CM Template";
    case 1:
      return "CG Template";
    case 2:
      return "DG Template";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & mdf::CaFlag::DynamicSize) {
    s << "Dynamic";
  }
  if (flag & mdf::CaFlag::InputQuantity) {
    s << (s.str().empty() ? "Input" : ",Input");
  }
  if (flag & mdf::CaFlag::OutputQuantity) {
    s << (s.str().empty() ? "Output" : ",Output");
  }
  if (flag & mdf::CaFlag::ComparisonQuantity) {
    s << (s.str().empty() ? "Comparison" : ",Comparison");
  }
  if (flag & mdf::CaFlag::Axis) {
    s << (s.str().empty() ? "Axis" : ",Axis");
  }
  if (flag & mdf::CaFlag::FixedAxis) {
    s << (s.str().empty() ? "Fixed" : ",Fixed");
  }
  if (flag & mdf::CaFlag::InverseLayout) {
    s << (s.str().empty() ? "Inverse" : ",Inverse");
  }
  if (flag & mdf::CaFlag::LeftOpenInterval) {
    s << (s.str().empty() ? "Left-Open" : ",Left-Open");
  }

  return s.str();
}
}  // namespace
namespace mdf::detail {

void Ca4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Array Type", MakeTypeString(type_));
  dest.emplace_back("Storage Type", MakeStorageString(storage_));
  dest.emplace_back("Nof Dimensions", std::to_string(dimension_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Byte Offset", std::to_string(byte_offset_base_));
  dest.emplace_back("Interval Bit Position",
                    std::to_string(invalid_bit_pos_base_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Ca4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, type_);
  bytes += ReadNumber(file, storage_);
  bytes += ReadNumber(file, dimension_);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, byte_offset_base_);
  bytes += ReadNumber(file, invalid_bit_pos_base_);
  dim_size_list_.clear();
  for (uint16_t dd = 0; dd < dimension_; ++dd) {
    uint64_t size = 0;
    bytes += ReadNumber(file, size);
    dim_size_list_.push_back(size);
  }
  axis_value_list_.clear();
  if (flags_ & CaFlag::FixedAxis) {
    for (uint16_t dd = 0; dd < dimension_; ++dd) {
      uint64_t size = dim_size_list_[dd];
      for (uint64_t ss = 0; ss < size; ++ss) {
        double temp = 0;
        bytes += ReadNumber(file, temp);
        axis_value_list_.push_back(temp);
      }
    }
  }

  // TODO(ihedvall): Figure out how the cycle count actually works
  return bytes;
}
int64_t Ca4Block::Index() const { return FilePosition(); }

void Ca4Block::Type(ArrayType type) { type_ = static_cast<uint8_t>(type); }

ArrayType Ca4Block::Type() const { return static_cast<ArrayType>(type_); }

void Ca4Block::Storage(ArrayStorage storage) {
  storage_ = static_cast<uint8_t>(storage);
}

ArrayStorage Ca4Block::Storage() const {
  return static_cast<ArrayStorage>(storage_);
}

void Ca4Block::Flags(uint32_t flags) { flags_ = flags; }

uint32_t Ca4Block::Flags() const { return flags_; }

Ca4Block::Ca4Block() { block_type_ = "##CA"; }

}  // namespace mdf::detail