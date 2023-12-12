/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ca4block.h"
#include <algorithm>

#include "cn4block.h"

namespace {

constexpr size_t kIndexComposition = 0;
constexpr size_t kIndexArray = 1;

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
      return "CN Template";
    case 1:
      return "CG Template";
    case 2:
      return "DG Template";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint32_t flag) {
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
    // Set up all the link list
  const auto nof_links = link_list_.size();
  const auto nof_data_blocks = Storage() == ArrayStorage::DgTemplate ? cycle_count_list_.size() : 0;
  const auto nof_dynamic_sizes = (Flags() & CaFlag::DynamicSize) != 0 ? 3 * dimensions_ : 0;
  const auto nof_input_quantities = (Flags() & CaFlag::InputQuantity) != 0 ? 3 * dimensions_ : 0;
  const auto nof_output_quantities = (Flags() & CaFlag::OutputQuantity) != 0 ? 3 * dimensions_ : 0;
  const auto nof_comp_quantities = (Flags() & CaFlag::ComparisonQuantity) != 0 ? 3 : 0;
  const auto nof_axis_conversions = (Flags() & CaFlag::Axis) != 0 ? dimensions_ : 0;
  const auto nof_axis = ((Flags() & CaFlag::Axis) != 0 ) && ((Flags() & CaFlag::FixedAxis) == 0) ? 3 * dimensions_ : 0;

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);

  size_t max_index;
  const auto* hd_block = HeaderBlock(); // Need header block so the function Find() search from there.
  for (size_t link_index = 0; link_index < nof_links && hd_block != nullptr; ++link_index) {
    std::ostringstream desc;
    const auto index = Link(link_index);
    const auto* block = hd_block->Find(index);

    if (link_index == kIndexComposition) {
      if (block != nullptr) {
        desc << "Link to " << block->BlockType() << " block";
      } else {
        desc << "Link to composition block";
      }
      dest.emplace_back("Composition Block", ToHexString(index),
                         desc.str(), BlockItemType::LinkItem);
      continue;
    }

    // DATA LINKS
    max_index = kIndexArray + nof_data_blocks;
    if (link_index < max_index) {
      if (block != nullptr) {
          desc << "Link to " << block->BlockType() << " block";
      } else {
          desc << "Link to data block";
      }
      dest.emplace_back("Data Block", ToHexString(index),
                        desc.str(), BlockItemType::LinkItem);
      continue;
    }

    // DYNAMIC LINKS
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes;
      if (link_index < max_index) {
          if (block != nullptr) {
              desc << "Link to " << block->BlockType() << " block";
          } else {
              desc << "Link to dynamic size block";
          }
          dest.emplace_back("Dynamic Size Block ", ToHexString(index),
                            desc.str(), BlockItemType::LinkItem);
          continue;
      }

    // INPUT QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities;
    if (link_index < max_index) {
      if (block != nullptr) {
        desc << "Link to " << block->BlockType() << " block";
      } else {
        desc << "Link to input quantity block";
      }
      dest.emplace_back("InputQuantity Block ", ToHexString(index),
                        desc.str(), BlockItemType::LinkItem);
      continue;
    }

    // OUTPUT QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities;
    if (link_index < max_index) {
      if (block != nullptr) {
        desc << "Link to " << block->BlockType() << " block";
      } else {
        desc << "Link to output quantity block";
      }
      dest.emplace_back("Output Quantity Block ", ToHexString(index),
                        desc.str(), BlockItemType::LinkItem);
      continue;
    }

    // COMPARISON QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities;
    if (link_index < max_index) {
      if (block != nullptr) {
        desc << "Link to " << block->BlockType() << " block";
      } else {
        desc << "Link to comparison quantity block";
      }
      dest.emplace_back("Comparison Quantity Block ", ToHexString(index),
                        desc.str(), BlockItemType::LinkItem);
      continue;
    }

    // AXIS CONVERSION
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities + nof_axis_conversions;
    if (link_index < max_index) {
      if (block != nullptr) {
        desc << "Link to " << block->BlockType() << " block";
      } else {
        desc << "Link to axis conversion block";
      }
      dest.emplace_back("Axis Conversion Block ", ToHexString(index),
                        desc.str(), BlockItemType::LinkItem);
      continue;
    }

    // AXIS
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities + nof_axis_conversions + nof_axis;
    if (link_index < max_index) {
      if (block != nullptr) {
        desc << "Link to " << block->BlockType() << " block";
      } else {
        desc << "Link to axis block";
      }
      dest.emplace_back("Axis Block ", ToHexString(index),
                        desc.str(), BlockItemType::LinkItem);
      continue;
    }
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Array Type", MakeTypeString(type_));
  dest.emplace_back("Storage Type", MakeStorageString(storage_));
  dest.emplace_back("Nof Dimensions", std::to_string(dimensions_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Byte Offset", std::to_string(byte_offset_base_));
  dest.emplace_back("Interval Bit Position",
                    std::to_string(invalid_bit_pos_base_));
   if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
  for (size_t dim = 0; dim < dim_size_list_.size(); ++dim) {
    std::ostringstream label;
    label << "Dimension " << dim << " Size:";
    dest.emplace_back(label.str(), std::to_string(dim_size_list_[dim]));
  }
  for (size_t axis = 0; axis < axis_value_list_.size(); ++axis) {
    std::ostringstream label;
    label << "Axis " << axis << ":";
    dest.emplace_back(label.str(), std::to_string(axis_value_list_[axis]));
  }
  for (size_t cycle = 0; cycle < cycle_count_list_.size(); ++cycle) {
    std::ostringstream label;
    label << "Cycle Count " << cycle << ":";
    dest.emplace_back(label.str(), std::to_string(cycle_count_list_[cycle]));
  }
}


size_t Ca4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file); // This function also read in all links
  bytes += ReadNumber(file, type_);
  bytes += ReadNumber(file, storage_);
  bytes += ReadNumber(file, dimensions_);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, byte_offset_base_);
  bytes += ReadNumber(file, invalid_bit_pos_base_);
  dim_size_list_.resize(dimensions_); // Resize number values/dimesion

  for (uint16_t dimension = 0; dimension < dimensions_; ++dimension) {
    bytes += ReadNumber(file, dim_size_list_[dimension]);
  }


  if (flags_ & CaFlag::FixedAxis) {
    const uint64_t sum_dim = NumberOfAxisValues();
    axis_value_list_.resize(sum_dim, 0.0);
    for (uint16_t index = 0; index < sum_dim; ++index) {
      bytes += ReadNumber(file, axis_value_list_[index]);
    }
  }

  switch (Storage()) {
    case ArrayStorage::CgTemplate:
    case ArrayStorage::DgTemplate:{
      // Be lazy and calculate number of bytes left and divide it with 8.
      const auto max_bytes = BlockLength();
      if (bytes < max_bytes) {
        const uint64_t max_count = (max_bytes - bytes) / 8;
        cycle_count_list_.resize(max_count);
        for (uint64_t index = 0; index < max_count; ++index) {
          bytes += ReadNumber(file, cycle_count_list_[index]);
        }
      }
      break;
    }

    default:
      break;
  }

  // Need to read all composition blocks if nay exist
  if (Link(kIndexComposition) > 0) {
    SetFilePosition(file, Link(kIndexComposition));
    const auto block_type = ReadBlockType(file);

    if (composition_list_.empty() && (Link(kIndexComposition) > 0)) {
      for (auto link = Link(kIndexComposition); link > 0; /* No ++ here*/) {
        if (block_type == "CA") {
          auto ca_block = std::make_unique<Ca4Block>();
          ca_block->Init(*this);
          SetFilePosition(file, link);
          ca_block->Read(file);
          link = ca_block->Link(0);
          composition_list_.emplace_back(std::move(ca_block));
        } else if (block_type == "CN") {
          auto cn_block = std::make_unique<Cn4Block>();
          cn_block->Init(*this);
          SetFilePosition(file, link);
          cn_block->Read(file);
          link = cn_block->Link(0);
          composition_list_.emplace_back(std::move(cn_block));
        }
      };
    }
  }
  return bytes;
}

int64_t Ca4Block::Index() const { return FilePosition(); }

void Ca4Block::Type(ArrayType type) { type_ = static_cast<uint8_t>(type); }

ArrayType Ca4Block::Type() const { return static_cast<ArrayType>(type_); }

std::string Ca4Block::TypeAsString() const {
  return MakeTypeString(type_);
}

void Ca4Block::Storage(ArrayStorage storage) {
  storage_ = static_cast<uint8_t>(storage);
}

ArrayStorage Ca4Block::Storage() const {
  return static_cast<ArrayStorage>(storage_);
}

std::string Ca4Block::StorageAsString() const {
  return MakeStorageString(storage_);
}


void Ca4Block::Flags(uint32_t flags) { flags_ = flags; }

uint32_t Ca4Block::Flags() const { return flags_; }

Ca4Block::Ca4Block() { block_type_ = "##CA"; }

uint64_t Ca4Block::NumberOfAxisValues() const {
  uint64_t sum = 0;
  std::for_each(dim_size_list_.cbegin(), dim_size_list_.cend(),
                [&] (auto dimension) {sum += dimension;});
  return sum;
}

const MdfBlock *Ca4Block::Find(int64_t index) const {

  for (const auto& composition_block : composition_list_) {
    if (!composition_block) {
      continue;
    }
    const auto* block = composition_block->Find(index);
    if (block != nullptr) {
      return block;
    }
  }

  return MdfBlock::Find(index);
}

uint16_t Ca4Block::Dimensions() const {
  return dimensions_;
}

void Ca4Block::DimensionSize(uint16_t dimension, uint64_t dimension_size) {
  while ( dimension >= dim_size_list_.size() ) {
    dim_size_list_.push_back(0);
  }
  dim_size_list_[dimension] = dimension_size;
}

uint64_t Ca4Block::DimensionSize(uint16_t dimension) const {
  return static_cast<size_t>(dimension) < dim_size_list_.size() ?
    dim_size_list_[dimension] : 0;
}

uint64_t Ca4Block::AxisValues() const {
  return static_cast<uint64_t>(axis_value_list_.size());
}

void Ca4Block::AxisValue(uint64_t index , double value) {
  while ( static_cast<size_t>(index) >= axis_value_list_.size() ) {
    axis_value_list_.push_back(0.0);
  }
  axis_value_list_[index] = value;
}

double Ca4Block::AxisValue(uint64_t index) const {
  return static_cast<size_t>(index) < axis_value_list_.size() ?
    axis_value_list_[index] : 0.0;
}


uint64_t Ca4Block::CycleCounts() const {
  return static_cast<uint64_t>(cycle_count_list_.size());
}

void Ca4Block::CycleCountOffset(uint64_t cycle_count, uint64_t offset) {
  while ( static_cast<size_t>(cycle_count) >= cycle_count_list_.size() ) {
    cycle_count_list_.push_back(0);
  }
  cycle_count_list_[cycle_count] = offset;
}

uint64_t Ca4Block::CycleCountOffset(uint64_t cycle_count) const {
  return static_cast<size_t>(cycle_count) < cycle_count_list_.size() ?
         cycle_count_list_[cycle_count] : 0;
}
void Ca4Block::CreateLinkLists() {
  // Set up all the link list

  const auto nof_links = link_list_.size();
  const auto nof_data_blocks = Storage() == ArrayStorage::DgTemplate ? cycle_count_list_.size() : 0;
  const auto nof_dynamic_sizes = (Flags() & CaFlag::DynamicSize) != 0 ? 3 * dimensions_ : 0;
  const auto nof_input_quantities = (Flags() & CaFlag::InputQuantity) != 0 ? 3 * dimensions_ : 0;
  const auto nof_output_quantities = (Flags() & CaFlag::OutputQuantity) != 0 ? 3 * dimensions_ : 0;
  const auto nof_comp_quantities = (Flags() & CaFlag::ComparisonQuantity) != 0 ? 3 : 0;
  const auto nof_axis_conversions = (Flags() & CaFlag::Axis) != 0 ? dimensions_ : 0;
  const auto nof_axis = ((Flags() & CaFlag::Axis) != 0 ) && ((Flags() & CaFlag::FixedAxis) == 0) ? 3 * dimensions_ : 0;
  data_links_.clear();
  dynamic_size_list_.clear();


  size_t triple_count = 0;
  CaTripleReference triple;
  const auto* hd_block = HeaderBlock(); // Need header block so the function Find() search from there.
  for (size_t link_index = kIndexArray; link_index < nof_links && hd_block != nullptr; ++link_index) {
    const auto index = Link(link_index);
    const auto* block = index > 0 ? hd_block->Find(index) : nullptr;

    // DATA LINKS
    size_t max_index = kIndexArray + nof_data_blocks;
    if (link_index < max_index) {
      data_links_.push_back(index);
      triple_count = 0;
      continue;
    }

    // DYNAMIC LINKS
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes;
    if (link_index < max_index) {
      switch (triple_count) {
        case 0:
          triple.DataGroup = block != nullptr ? dynamic_cast<const IDataGroup*>(block) : nullptr;
          break;

        case 1:
          triple.ChannelGroup = block != nullptr ? dynamic_cast<const IChannelGroup*>(block) : nullptr;
          break;

        case 2:
          triple.Channel = block != nullptr ? dynamic_cast<const IChannel*>(block) : nullptr;
          break;

        default:
          break;
      }
      ++triple_count;
      if (triple_count >= 3) {
        dynamic_size_list_.emplace_back(triple);
        triple = {};
        triple_count = 0;
      }
      continue;
    }

    // INPUT QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities;
    if (link_index < max_index) {
      switch (triple_count) {
        case 0:
          triple.DataGroup = block != nullptr ? dynamic_cast<const IDataGroup*>(block) : nullptr;
          break;

        case 1:
          triple.ChannelGroup = block != nullptr ? dynamic_cast<const IChannelGroup*>(block) : nullptr;
          break;

        case 2:
          triple.Channel = block != nullptr ? dynamic_cast<const IChannel*>(block) : nullptr;
          break;

        default:
          break;
      }
      ++triple_count;
      if (triple_count >= 3) {
        input_quantity_list_.emplace_back(triple);
        triple = {};
        triple_count = 0;
      }
      continue;
    }

    // OUTPUT QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities;
    if (link_index < max_index) {
      switch (triple_count) {
        case 0:
          triple.DataGroup = block != nullptr ? dynamic_cast<const IDataGroup*>(block) : nullptr;
          break;

        case 1:
          triple.ChannelGroup = block != nullptr ? dynamic_cast<const IChannelGroup*>(block) : nullptr;
          break;

        case 2:
          triple.Channel = block != nullptr ? dynamic_cast<const IChannel*>(block) : nullptr;
          break;

        default:
          break;
      }
      ++triple_count;
      if (triple_count >= 3) {
        output_quantity_list_.emplace_back(triple);
        triple = {};
        triple_count = 0;
      }
      continue;
    }

    // COMPARISON QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities;
    if (link_index < max_index) {
      switch (triple_count) {
        case 0:
          triple.DataGroup = block != nullptr ? dynamic_cast<const IDataGroup*>(block) : nullptr;
          break;

        case 1:
          triple.ChannelGroup = block != nullptr ? dynamic_cast<const IChannelGroup*>(block) : nullptr;
          break;

        case 2:
          triple.Channel = block != nullptr ? dynamic_cast<const IChannel*>(block) : nullptr;
          break;

        default:
          break;
      }
      ++triple_count;
      if (triple_count >= 3) {
        comparison_quantity_list_.emplace_back(triple);
        triple = {};
        triple_count = 0;
      }
      continue;
    }

    // AXIS CONVERSION
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities + nof_axis_conversions;
    if (link_index < max_index) {
      const auto* conversion = block != nullptr ? dynamic_cast<const IChannelConversion*>(block) : nullptr;
      axis_conversion_list_.emplace_back(conversion);
      triple_count = 0;
      continue;
    }

    // AXIS
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities + nof_axis_conversions + nof_axis;
    if (link_index < max_index) {
      switch (triple_count) {
        case 0:
          triple.DataGroup = block != nullptr ? dynamic_cast<const IDataGroup*>(block) : nullptr;
          break;

        case 1:
          triple.ChannelGroup = block != nullptr ? dynamic_cast<const IChannelGroup*>(block) : nullptr;
          break;

        case 2:
          triple.Channel = block != nullptr ? dynamic_cast<const IChannel*>(block) : nullptr;
          break;

        default:
          break;
      }
      ++triple_count;
      if (triple_count >= 3) {
        axis_list_.emplace_back(triple);
        triple = {};
        triple_count = 0;
      }
      continue;
    }
  }

}

}  // namespace mdf::detail