/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "sr4block.h"
#include "mdf/ichannelgroup.h"
#include "datablock.h"
#include "cg4block.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexData = 1;

std::string MakeSyncTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "";
    case 1:
      return "Timer";
    case 2:
      return "Angle";
    case 3:
      return "Distance";
    case 4:
      return "Index";
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

}  // namespace

namespace mdf::detail {

Sr4Block::Sr4Block() {
  block_type_ = "##SR";
}

int64_t Sr4Block::Index() const {
  return MdfBlock::Index();
}

std::string Sr4Block::BlockType() const {
  return MdfBlock::BlockType();
}

void Sr4Block::NofSamples(uint64_t nof_samples) {
  nof_samples_ = nof_samples;
}

uint64_t Sr4Block::NofSamples() const {
  return nof_samples_;
}

void Sr4Block::Interval(double interval) {
  interval_ = interval;
}

double Sr4Block::Interval() const {
  return interval_;
}

void Sr4Block::SyncType(SrSyncType type) {
  type_ = static_cast<uint8_t>(type);
}

SrSyncType Sr4Block::SyncType() const {
  return static_cast<SrSyncType>(type_);
}

void Sr4Block::Flags(uint8_t flags) {
  flags_ = flags;
}

uint8_t Sr4Block::Flags() const {
  return flags_;
}

const IChannelGroup *Sr4Block::ChannelGroup() const {
  const auto* mdf_block = CgBlock();
  return mdf_block != nullptr ? dynamic_cast<const IChannelGroup*>(mdf_block) : nullptr;
}

void Sr4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next SR", ToHexString(Link(kIndexNext)),
                    "Link to next sample reduction", BlockItemType::LinkItem);
  dest.emplace_back("Data", ToHexString(Link(kIndexData)), "Link to data",
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

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

void Sr4Block::ReadData(std::FILE *file) const {
  const size_t count = DataSize();
  if (block_list_.empty() || count == 0) {
    data_list_.clear();
    return;
  }
  try {
    data_list_.resize(count, 0);
  } catch (const std::exception& ) {
    // No room in memory for the block.
    data_list_.clear();
    return;
  }
  size_t index = 0;


  // The block list should only contain one block.
  // A SD block is uncompressed data block
  for (const auto &block : block_list_) {
    const auto *list = dynamic_cast<const DataListBlock *>(block.get());
    const auto *data = dynamic_cast<const DataBlock *>(block.get());
    if (list != nullptr) {
      list->CopyDataToBuffer( file, data_list_, index);
    } else if (block != nullptr) {
      data->CopyDataToBuffer(file, data_list_, index);
    }
  }
}

void Sr4Block::ClearData() {
  DataListBlock::ClearData();
  data_list_.clear();
}

void Sr4Block::GetChannelValueUint( const IChannel& channel, uint64_t sample,
                          uint64_t array_index, SrValue<uint64_t>& value ) const {
  GetChannelValueT(channel, sample, array_index,value);
}

void Sr4Block::GetChannelValueInt( const IChannel& channel, uint64_t sample,
                                    uint64_t array_index, SrValue<int64_t>& value ) const {
  GetChannelValueT(channel, sample, array_index,value);}

void Sr4Block::GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                                   uint64_t array_index, SrValue<double>& value ) const {
  GetChannelValueT(channel, sample, array_index,value);
}



}  // namespace mdf::detail