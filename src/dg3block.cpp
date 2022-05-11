/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <algorithm>
#include "dg3block.h"
namespace {
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCg = 1;
constexpr size_t kIndexTr = 2;
constexpr size_t kIndexData = 3;
}
namespace mdf::detail {

const IBlock *Dg3Block::Find(fpos_t index) const {
  if (tr_block_) {
    const auto *pos = tr_block_->Find(index);
    if (pos != nullptr) {
      return pos;
    }
  }
  for (const auto &cg3: cg_list_) {
    if (!cg3) {
      continue;
    }
    const auto *pos = cg3->Find(index);
    if (pos != nullptr) {
      return pos;
    }
  }
  return DataListBlock::Find(index);
}

void Dg3Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next DG", ToHexString(Link(kIndexNext)), "Link to next data group", BlockItemType::LinkItem );
  dest.emplace_back("First CG", ToHexString(Link(kIndexCg)), "Link to first channel group",BlockItemType::LinkItem );
  dest.emplace_back("Link Data", ToHexString(Link(kIndexTr)), "Link to trigger data", BlockItemType::LinkItem );
  dest.emplace_back("Link Data", ToHexString(Link(kIndexData)), "Link to Data", BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Channel Groups", std::to_string(nof_cg_blocks_));
  dest.emplace_back("Record ID size [bytes]", std::to_string(nof_record_id_));
}

size_t Dg3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadLinks3(file, 4);
  bytes += ReadNumber(file, nof_cg_blocks_);
  bytes += ReadNumber(file, nof_record_id_);

  if (Link(kIndexTr) > 0) {
    tr_block_ = std::make_unique<Tr3Block>();
    tr_block_->Init(*this);
    SetFilePosition(file, Link(kIndexTr));
    tr_block_->Read(file);
  }

  if (Link(kIndexData) > 0) {
    auto dt3 = std::make_unique<Dt3Block>();
    dt3->Init(*this);
    SetFilePosition(file, Link(kIndexData));
    dt3->Read(file);
    block_list_.push_back(std::move(dt3));
  }

  cg_list_.clear();
  for (auto link = Link(kIndexCg); link > 0; /* No ++ here*/) {
    auto cg3 = std::make_unique<Cg3Block>();
    cg3->Init(*this);
    SetFilePosition(file, link);
    cg3->Read(file);
    link = cg3->Link(kIndexNext);
    cg_list_.push_back(std::move(cg3));
  }

  return bytes;
}

size_t Dg3Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0; // Write or update the values inside the block
  nof_cg_blocks_ = static_cast<uint16_t>(cg_list_.size());
  nof_record_id_ = nof_cg_blocks_ > 1 ? 1 : 0;
  if (!update) {
    block_type_ = "DG";
    block_size_ = (2 + 2) + (4*4) + 2 + 2 + 4;
    link_list_.resize(4,0);
  }

  size_t bytes = update ? IBlock::Update(file) : IBlock::Write(file);
  bytes += WriteNumber(file, nof_cg_blocks_);
  bytes += WriteNumber(file, nof_record_id_);
  const std::vector<uint8_t> reserved(4,0);
  bytes += WriteByte(file,reserved);

  if (tr_block_ && Link(kIndexTr) <= 0) {
    tr_block_->Write(file);
    UpdateLink(file, kIndexTr, tr_block_->FilePosition());
  }

  for (size_t cg_index = 0; cg_index < cg_list_.size(); ++cg_index) {
    auto& cg3 = cg_list_[cg_index];
    if (!cg3) {
      continue;
    }
    cg3->Write(file);
    if (cg_index == 0) {
      UpdateLink(file, kIndexCg, cg3->FilePosition());
    } else {
      auto& prev = cg_list_[cg_index - 1];
      if (prev) {
        prev->UpdateLink(file, kIndexNext, cg3->FilePosition());
      }
    }
  }

  for (size_t ii = 0; ii < block_list_.size(); ++ii) {
    auto& data = block_list_[ii];
    if (!data) {
      continue;
    }
    if (data->FilePosition() > 0) {
      continue;
    }
    data->Write(file);
    if (ii == 0) {
      UpdateLink(file, kIndexData, data->FilePosition());
    }
  }

  return bytes;
}

void Dg3Block::AddCg3(std::unique_ptr<Cg3Block> &cg3) {
  cg3->Init(*this);
  cg3->RecordId(0);
  cg_list_.push_back(std::move(cg3));
  nof_cg_blocks_ = static_cast<uint16_t> (cg_list_.size());
  nof_record_id_ = cg_list_.size() > 1 ? 1 : 0;
  uint8_t id3 = cg_list_.size() < 2 ? 0 : 1;
  std::ranges::for_each(cg_list_, [&] (auto& group) {group->RecordId(id3++);});
}

const Dg3Block::Cg3List &Dg3Block::Cg3() const {
  return cg_list_;
}

int64_t Dg3Block::Index() const {
  return FilePosition();
}

std::string Dg3Block::Description() const {
  return {};
}

std::vector<IChannelGroup *> Dg3Block::ChannelGroups() const {
  std::vector<IChannelGroup*> list;
  for (const auto& cg3 : cg_list_) {
    if (cg3) {
      list.emplace_back(cg3.get());
    }
  }
  return list;
}

void Dg3Block::ReadData(std::FILE *file) const {
  if (file == nullptr) {
    throw std::invalid_argument("File pointer is null");
  }
  std::FILE* data_file = nullptr;
  size_t data_size = DataSize();
  SetFilePosition(file, Link(kIndexData));
  data_file = file;

  auto pos = GetFilePosition(data_file);
  // Read through all record
  ParseDataRecords(data_file, data_size);
}

void Dg3Block::ParseDataRecords(std::FILE *file, size_t nof_data_bytes) const {
  if (file == nullptr || nof_data_bytes == 0) {
    return;
  }
  ResetSample();

  for (size_t count = 0; count < nof_data_bytes; /* No ++count here*/) {
    // 1. Read Record ID
    uint8_t record_id = 0;
    if (nof_record_id_ == 1 || nof_record_id_ == 2) {
      count += ReadNumber(file,record_id);
    }
    const auto* cg3 = FindCgRecordId(record_id);
    if (cg3 == nullptr) {
      break;
    }

    const auto read = cg3->ReadDataRecord(file, *this);
    if (read == 0) {
      break;
    }
    count += read;
    if (nof_record_id_ == 2) {
      count += ReadNumber(file,record_id);
    }
  }
}

const Cg3Block *Dg3Block::FindCgRecordId(const uint64_t record_id) const {
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

} // end namespace mdf::detail