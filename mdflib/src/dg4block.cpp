/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dg4block.h"

#include <stdexcept>
#include <algorithm>

#include "dl4block.h"
#include "dt4block.h"
#include "dz4block.h"
#include "hl4block.h"
#include "mdf/mdflogstream.h"

namespace {
constexpr size_t kIndexCg = 1;
constexpr size_t kIndexData = 2;
constexpr size_t kIndexMd = 3;
constexpr size_t kIndexNext = 0;

///< Helper function that recursively copies all data bytes to a
/// destination file.
size_t CopyDataToFile(
    const mdf::detail::DataListBlock::BlockList& block_list,  // NOLINT
    std::FILE* from_file, std::FILE* to_file) {
  size_t count = 0;
  for (const auto& block : block_list) {
    if (!block) {
      continue;
    }
    const auto* db = dynamic_cast<const mdf::detail::DataBlock*>(block.get());
    const auto* dl =
        dynamic_cast<const mdf::detail::DataListBlock*>(block.get());
    if (db != nullptr) {
      count += db->CopyDataToFile(from_file, to_file);
    } else if (dl != nullptr) {
      count += CopyDataToFile(dl->DataBlockList(), from_file, to_file);
    }
  }
  return count;
}

}  // namespace

namespace mdf::detail {

Dg4Block::Dg4Block() { block_type_ = "##DG"; }
IChannelGroup* Dg4Block::CreateChannelGroup() {
  auto cg4 = std::make_unique<Cg4Block>();
  cg4->Init(*this);
  AddCg4(cg4);
  return cg_list_.empty() ? nullptr : cg_list_.back().get();
}

const MdfBlock* Dg4Block::Find(int64_t index) const {
  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    const auto* p = cg->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return DataListBlock::Find(index);
}

void Dg4Block::GetBlockProperty(BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next DG", ToHexString(Link(kIndexNext)),
                    "Link to next data group", BlockItemType::LinkItem);
  dest.emplace_back("First CG", ToHexString(Link(kIndexCg)),
                    "Link to first channel group", BlockItemType::LinkItem);
  dest.emplace_back("Link Data", ToHexString(Link(kIndexData)), "Link to Data",
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Record ID Size [byte]", std::to_string(rec_id_size_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Dg4Block::Read(std::FILE* file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, rec_id_size_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 7);

  ReadMdComment(file, kIndexMd);
  ReadBlockList(file, kIndexData);
  return bytes;
}

size_t Dg4Block::Write(std::FILE* file) {
  const bool update = FilePosition() > 0;  // True if already written to file
  if (!update) {
    block_type_ = "##DG";
    block_length_ = 24 + (4 * 8) + 8;
    link_list_.resize(4, 0);
  }

  WriteLink4List(file, cg_list_, kIndexCg,
              UpdateOption::DoUpdateAllBlocks); // Save nof samples in CG block
  WriteMdComment(file, kIndexMd);


  auto bytes = update ? MdfBlock::Update(file) : MdfBlock::Write(file);
  if (update) {
    bytes = block_length_;
  } else {
    bytes += WriteNumber(file, rec_id_size_);
    bytes += WriteBytes(file, 7);
    UpdateBlockSize(file, bytes);
    // Need to update the signal data link for any chaannels referencing
    // VLSD CG bloks.
    for (auto& cg4 : cg_list_) {
      if (cg4 && (cg4->Flags() & CgFlag::VlsdChannel) ) {
        // The previous CG block have a channel which need to updates its
        // signal data link.
        auto* other_cg4 = FindCgRecordId(cg4->RecordId() -1);
        auto* cn4 = other_cg4 != nullptr ?
                 other_cg4->FindVlsdChannel(cg4->RecordId()) : nullptr;
        if (cn4 != nullptr) {
          cn4->UpdateDataLink(file, cg4->FilePosition());
        }
      }
    }
  }
  // Need to write any data block so it is positioned last in the file
  // as any DT block should be appended with data bytes. The DT block must be
  // last in the file
  WriteLink4List(file, block_list_, kIndexData,
                 UpdateOption::DoUpdateAllBlocks); // Update last HL or DT
  return bytes;
}
size_t Dg4Block::DataSize() const { return DataListBlock::DataSize(); }

void Dg4Block::ReadCgList(std::FILE* file) {
  ReadLink4List(file, cg_list_, kIndexCg);
}

void Dg4Block::ReadData(std::FILE* file) const {
  if (file == nullptr) {
    throw std::invalid_argument("File pointer is null");
  }
  const auto& block_list = DataBlockList();
  if (block_list.empty()) {
    return;
  }

  // First scan through all CN blocks and set up any VLSD CG or MLSD channel
  // relations.

  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    const auto channel_list = cg->Channels();
    for (const auto* channel :channel_list) {
      if (channel == nullptr) {
        continue;
      }
      const auto* cn_block = dynamic_cast<const Cn4Block*>(channel);
      if (cn_block == nullptr) {
        continue;
      }
      // Fetch the channels referenced data block. Note that some type of
      // data blocks are own by this channel as SD block but some are only
      // references to block own by another block. Of interest is VLSD CG block
      // and MLSD channel.
      const auto data_link = cn_block->DataLink();
      if (data_link == 0) {
        continue; // No data to read into the system
      }
      const auto* block = Find(data_link);
      if (block == nullptr) {
        MDF_DEBUG() << "Missing data block in channel. Channel :"
                    << cn_block->Name()
                    << ", Data Link: " << data_link;

        continue; // Strange that the block doesn't exist
      }

      switch (cn_block->Type()) {
        case ChannelType::VariableLength:
          if (block->BlockType() == "CG") {
            const auto* cg_block = dynamic_cast<const Cg4Block*>(block);
            if (cg_block != nullptr &&
                (cg_block->Flags() & CgFlag::VlsdChannel) != 0 ) {
              cn_block->VlsdRecordId(cg_block->RecordId());
            } else {
              cn_block->VlsdRecordId(0);
              cn_block->ReadData(file);
            }
          } else if (block->BlockType() == "SD") {
            cn_block->ReadData(file);
          } else if (block->BlockType() == "DZ") {
            cn_block->ReadData(file);
          } else if (block->BlockType() == "DL") {
            cn_block->ReadData(file);          }
          break;

        case ChannelType::MaxLength:
          if (block->BlockType() == "CN") {
            // Point to the length of byte array channel
            const auto* mlsd_channel = dynamic_cast<const Cn4Block*>(block);
            if (mlsd_channel != nullptr) {
              cn_block->MlsdChannel(mlsd_channel);
            } else {
              cn_block->MlsdChannel(nullptr);
            }
          }
          break;

        default:
          break;
      }
    }
  }

  // Convert everything to a samples in a file single DT block can be read
  // directly but remaining block types are streamed to a temporary file. The
  // main reason is that linked data blocks,is not aligned to a record or even
  // worse a channel value bytes. Converting everything to a simple DT block
  // solves that problem.

  bool close_data_file = false; // If DT block do not close the data file.
  std::FILE* data_file = nullptr;
  size_t data_size = 0;
  if (block_list.size() == 1 && block_list[0] &&
      block_list[0]->BlockType() == "DT") {  // If DT read from file directly
    const auto* dt = dynamic_cast<const Dt4Block*>(block_list[0].get());
    if (dt != nullptr) {
      SetFilePosition(file, dt->DataPosition());
      data_file = file;
      data_size = dt->DataSize();
    }
  } else {
    // If not a DT block, extract the linked data block list into temporary
    // file which is a DT block according to the above DT block.
    close_data_file = true;
    data_file = std::tmpfile();
    data_size = CopyDataToFile(block_list, file, data_file);
    std::rewind(data_file);  // SetFilePosition(data_file,0);
  }

  // Now, it is time to scan through and do a call-back for each record
  // auto pos = GetFilePosition(data_file);
  // Read through all record
  ParseDataRecords(data_file, data_size);
  if (data_file != nullptr && close_data_file) {
    fclose(data_file);
  }

  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    for (const auto& cn : cg->Cn4()) {
      if (!cn) {
        continue;
      }
      cn->ClearData();
    }
  }
}

void Dg4Block::ParseDataRecords(std::FILE* file, size_t nof_data_bytes) const {
  if (file == nullptr || nof_data_bytes == 0) {
    return;
  }
  ResetSample();

  for (size_t count = 0; count < nof_data_bytes; /* No ++count here*/) {
    // 1. Read Record ID
    uint64_t record_id = 0;
    count += ReadRecordId(file, record_id);

    const auto* cg = FindCgRecordId(record_id);
    if (cg == nullptr) {
      break;
    }
    const auto read = cg->ReadDataRecord(file, *this);
    if (read == 0) {
      break;
    }

    count += read;
  }
}

size_t Dg4Block::ReadRecordId(std::FILE* file, uint64_t& record_id) const {
  record_id = 0;
  if (file == 0) {
    return 0;
  }
  size_t count = 0;
  switch (rec_id_size_) {
    case 1: {
      uint8_t id = 0;
      count += ReadNumber(file, id);
      record_id = id;
      break;
    }

    case 2: {
      uint16_t id = 0;
      count += ReadNumber(file, id);
      record_id = id;
      break;
    }

    case 4: {
      uint32_t id = 0;
      count += ReadNumber(file, id);
      record_id = id;
      break;
    }

    case 8: {
      uint64_t id = 0;
      count += ReadNumber(file, id);
      record_id = id;
      break;
    }
    default:
      break;
  }
  return count;
}

Cg4Block* Dg4Block::FindCgRecordId(uint64_t record_id) const {
  if (cg_list_.size() == 1) {
    return cg_list_[0].get();
  }
  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    if (cg->RecordId() == record_id) {
      return cg.get();
    }
  }
  return nullptr;
}

std::vector<IChannelGroup*> Dg4Block::ChannelGroups() const {
  std::vector<IChannelGroup*> list;
  for (const auto& cg : cg_list_) {
    if (cg) {
      list.emplace_back(cg.get());
    }
  }
  return list;
}

void Dg4Block::AddCg4(std::unique_ptr<Cg4Block>& cg4) {
  cg4->Init(*this);
  cg4->RecordId(0);
  cg_list_.push_back(std::move(cg4));

  if (cg_list_.size() < 2) {
    rec_id_size_ = 0;
  } else if (cg_list_.size() < 0x100) {
    rec_id_size_ = 1;
  } else if (cg_list_.size() < 0x10000) {
    rec_id_size_ = 2;
  } else if (cg_list_.size() < 0x100000000) {
    rec_id_size_ = 4;
  } else {
    rec_id_size_ = 8;
  }

  uint64_t id4 = cg_list_.size() < 2 ? 0 : 1;
  for (auto& group : cg_list_) {
    if (group) {
      group->RecordId(id4++);
    }
  }
}

int64_t Dg4Block::Index() const { return FilePosition(); }

IMetaData* Dg4Block::CreateMetaData() {
  return MdfBlock::CreateMetaData();
}

IMetaData* Dg4Block::MetaData() const {
  return MdfBlock::MetaData();
}

void Dg4Block::Description(const std::string& desc) {
  auto* md4 = CreateMetaData();
  if (md4 != nullptr) {
    md4->StringProperty("TX", desc);
  }
}
std::string Dg4Block::Description() const {
  const auto* md4 = MetaData();
  return md4 == nullptr ? std::string() : md4->StringProperty("TX");
}

void Dg4Block::RecordIdSize(uint8_t id_size) { rec_id_size_ = id_size; }

uint8_t Dg4Block::RecordIdSize() const { return rec_id_size_; }

bool Dg4Block::UpdateDtBlocks(std::FILE *file) {
  auto& block_list = DataBlockList();
  if (block_list.empty()) {
    // No data blocks to update
    MDF_DEBUG() << "No last data block to update.";
    return true;
  }
  auto* last_block = block_list.back().get();
  if (last_block == nullptr || last_block->BlockType() != "DT") {
    MDF_DEBUG() << "Last data block is not a DT block.";
    return true;
  }
  auto* dt_block = dynamic_cast<Dt4Block*>(last_block);
  if (dt_block == nullptr) {
    MDF_ERROR() << "Invalid DT block type-cast.";
    return false;
  }
  dt_block->UpdateDataSize(file);

  return true;
}

// Update the unfinished payload data (DT) block. This function update the
// channel group (CG) and a CG-VLSD channel group regarding cycle count
// and offsets (VLSD)
bool Dg4Block::UpdateCgAndVlsdBlocks(std::FILE* file, bool update_cg,
                                     bool update_vlsd) {
  auto& block_list = DataBlockList();
  if (block_list.empty()) {
    // No data blocks to update
    MDF_DEBUG() << "No last data block to update.";
    return true;
  }
  // It is the last DT block in the list that can be updated. If it isn't a DT
  // block, we cannot finish the file.
  auto* last_block = block_list.back().get();
  if (last_block == nullptr || last_block->BlockType() != "DT") {
    MDF_DEBUG() << "Last data block is not a DT block.";
    return true;
  }
  auto* dt_block = dynamic_cast<Dt4Block*>(last_block);
  if (dt_block == nullptr) {
    MDF_ERROR() << "Invalid DT block type-cast.";
    return false;
  }
  mdf::detail::SetFilePosition(file, dt_block->DataPosition());
  size_t count = 0;
  while (count < dt_block->DataSize()) {
    uint64_t record_id = 0;
    count += ReadRecordId(file,record_id);
    const auto* cg_block = FindCgRecordId(record_id);
    if (cg_block == nullptr) {
      MDF_DEBUG() << "Failed to find the CG block. Record ID: " << record_id;
      break;
    }
    auto* cg4 = const_cast<Cg4Block*>(cg_block);
    const auto vlsd = (cg4->Flags() & CgFlag::VlsdChannel) != 0;
    if (!vlsd && update_cg) {
      count += cg4->UpdateCycleCounter(file); // Increment the cycle counter
    } else if (vlsd && update_vlsd) {
      count += cg4->UpdateVlsdSize(file); // Update size and offset
    } else {
      count += cg4->StepRecord(file); //
    }
  }
  return true;
}

const IChannelGroup* Dg4Block::FindParentChannelGroup(const IChannel&
                                                      channel) const {
  const auto channel_index = channel.Index();
  const auto &cg_list = Cg4();
  const auto itr = std::find_if(cg_list.cbegin(), cg_list.cend(),
                                [&](const auto &cg_block) {
    return cg_block && cg_block->Find(channel_index) != nullptr;
  });
  return itr != cg_list.cend() ? itr->get() : nullptr;
}

}  // namespace mdf::detail