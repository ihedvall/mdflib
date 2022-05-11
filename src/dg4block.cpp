/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <stdexcept>
#include "dg4block.h"
#include "dt4block.h"
#include "dz4block.h"
#include "dl4block.h"
#include "hl4block.h"

namespace {
constexpr size_t kIndexCg = 1;
constexpr size_t kIndexData = 2;
constexpr size_t kIndexMd = 3;
constexpr size_t kIndexNext = 0;

///< Helper function that recursively copies all data bytes to a
/// destination file.
size_t CopyDataToFile(const mdf::detail::DataListBlock::BlockList& block_list,
                      std::FILE* from_file, std::FILE* to_file) {
  size_t count = 0;
  for (const auto& block : block_list) {
    if (!block) {
      continue;
    }
    const auto* db = dynamic_cast< const mdf::detail::DataBlock* > (block.get());
    const auto* dl = dynamic_cast< const mdf::detail::DataListBlock* > (block.get());
    if (db != nullptr) {
      count += db->CopyDataToFile(from_file, to_file);
    } else if (dl != nullptr) {
      count += CopyDataToFile(dl->DataBlockList(),from_file, to_file);
    }
  }
  return count;
}

}

namespace mdf::detail {

const IBlock *Dg4Block::Find(fpos_t index) const {
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

void Dg4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next DG", ToHexString(Link(kIndexNext)), "Link to next data group", BlockItemType::LinkItem );
  dest.emplace_back("First CG", ToHexString(Link(kIndexCg)), "Link to first channel group",BlockItemType::LinkItem );
  dest.emplace_back("Link Data", ToHexString(Link(kIndexData)), "Link to Data", BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Record ID Size [byte]", std::to_string(rec_id_size_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Dg4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, rec_id_size_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 7);

  ReadMdComment(file, kIndexMd);
  ReadBlockList(file, kIndexData);
  return bytes;
}

void Dg4Block::ReadCgList(std::FILE* file) {
  if (cg_list_.empty() && Link(kIndexCg) > 0) {
    for (auto link = Link(kIndexCg); link > 0; /* No ++ here*/) {
      auto cg = std::make_unique<Cg4Block>();
      cg->Init(*this);
      SetFilePosition(file, link);
      cg->Read(file);
      link = cg->Link(kIndexNext);
      cg_list_.emplace_back(std::move(cg));
    }
  }
}

void Dg4Block::ReadData(std::FILE *file) const {
  if (file == nullptr) {
    throw std::invalid_argument("File pointer is null");
  }
  const auto& block_list = DataBlockList();
  if (block_list.empty()) {
    return;
  }
  // First scan through all CN blocks and read in any VLSD data related data bytes into memory.

  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    for (const auto& cn : cg->Cn4()) {
      if (!cn) {
        continue;
      }
      cn->ReadData(file);
    }
  }


  // Convert everything to a samples in a file single DT block can be read directly but remaining
  // block types are streamed to a temporary file. The main reason is that linked data blocks not
  // is aligned to a record or even worse a channel value bytes. Converting everything to a simple
  // DT block solves that problem.

  bool close_data_file = false;
  std::FILE* data_file = nullptr;
  size_t data_size = 0;
  if ( block_list.size() == 1 && block_list[0] && block_list[0]->BlockType() == "DT") { // If DT read from file directly
    const auto* dt = dynamic_cast<const Dt4Block*> (block_list[0].get());
    if (dt != nullptr) {
      SetFilePosition(file, dt->DataPosition());
      data_file = file;
      data_size = dt->DataSize();
    }
  } else {
    close_data_file = true;
    data_file = std::tmpfile();
    data_size = CopyDataToFile(block_list, file, data_file);
    std::rewind(data_file); //SetFilePosition(data_file,0);
  }

  auto pos = GetFilePosition(data_file);
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

void Dg4Block::ParseDataRecords(std::FILE *file, size_t nof_data_bytes) const {
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

size_t Dg4Block::ReadRecordId(std::FILE *file, uint64_t &record_id) const {
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
    default: break;
  }
  return count;
}

const Cg4Block *Dg4Block::FindCgRecordId(const uint64_t record_id) const {
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

std::vector<IChannelGroup *> Dg4Block::ChannelGroups() const {
  std::vector<IChannelGroup*> list;
  for (const auto& cg : cg_list_) {
    if (cg) {
      list.emplace_back(cg.get());
    }
  }
  return list;
}

}