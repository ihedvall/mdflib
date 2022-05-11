/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <algorithm>
#include <ranges>
#include <cstdio>
#include "cg3block.h"
#include "mdf/idatagroup.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCn = 1;
constexpr size_t kIndexTx = 2;
constexpr size_t kIndexSr = 3;

}

namespace mdf::detail {

int64_t Cg3Block::Index() const {
  return FilePosition();
}

void Cg3Block::Name(const std::string &name) {
}

std::string Cg3Block::Name() const {
  return {};
}

void Cg3Block::Description(const std::string &description) {
  comment_ = description;
}
std::string Cg3Block::Description() const {
  return comment_;
}

uint64_t Cg3Block::NofSamples() const {
  return nof_records_;
}

void Cg3Block::NofSamples(uint64_t nof_samples) {
  nof_records_ = static_cast<uint32_t>(nof_samples);
}

void Cg3Block::RecordId(uint64_t record_id) {
  record_id_ = static_cast<uint16_t>(record_id);
}

uint64_t Cg3Block::RecordId() const {
  return record_id_;
}

std::vector<IChannel *> Cg3Block::Channels() const {
  std::vector<IChannel *> channel_list;
  std::ranges::for_each(cn_list_, [&] (const auto& cn) {channel_list.push_back(cn.get()); });
  return channel_list;
}

const IChannel *Cg3Block::GetXChannel(const IChannel&) const {
  // Search for the master channel in the group
  auto master = std::ranges::find_if(cn_list_, [&] (const auto& cn3) {
    return !cn3 ? false : cn3->Type() == ChannelType::Master || cn3->Type() == ChannelType::VirtualMaster;
  });

  return master != cn_list_.cend() ? master->get() : nullptr;
}

void Cg3Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CG", ToHexString(Link(kIndexNext)), "Link to next channel group", BlockItemType::LinkItem );
  dest.emplace_back("First CN", ToHexString(Link(kIndexCn)), "Link to first channel",BlockItemType::LinkItem );
  dest.emplace_back("Comment TX", ToHexString(Link(kIndexTx)), "Link to group comment", BlockItemType::LinkItem );
  dest.emplace_back("Reduction SR", ToHexString(Link(kIndexSr)), "Link to first sample reduction", BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Record ID", std::to_string(record_id_));
  dest.emplace_back("Nof Channels", std::to_string(nof_channels_));
  dest.emplace_back("Record Size [bytes]", std::to_string(size_of_data_record_));
  dest.emplace_back("Nof Samples", std::to_string(nof_records_));
  dest.emplace_back("Comment", comment_);
}

size_t Cg3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadLinks3(file, 3);

  bytes += ReadNumber(file, record_id_);
  bytes += ReadNumber(file, nof_channels_);
  bytes += ReadNumber(file, size_of_data_record_);
  bytes += ReadNumber(file, nof_records_);
  if (bytes + 4 < block_size_) {
    uint32_t link = 0;
    bytes += ReadNumber(file, link);
    link_list_.emplace_back(link);
  }
  comment_ = ReadTx3(file, kIndexTx);
  return bytes;
}

size_t Cg3Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0; // Write or update the values inside the block
  nof_channels_ = static_cast<uint16_t>(cn_list_.size());
  PrepareForWriting();
  if (!update) {
    block_type_ = "CG";
    block_size_ = (2 + 2) + (3*4) + 2 + 2 + 2 + 4 + 4;
    link_list_.resize(3,0); // Note that an extra link is added at the end of the block.
  }

  int64_t sr_link = 0; // This link is added at the end of the block not at the beginning
  for (size_t sr_index = 0; sr_index < sr_list_.size(); ++sr_index) {
    auto& sr3 = sr_list_[sr_index];
    if (!sr3 || sr3->FilePosition() > 0) {
      continue;
    }
    sr3->Write(file);
    if (sr_index == 0) {
      sr_link = sr3->FilePosition();
    } else {
      auto& prev = sr_list_[sr_index - 1];
      if (prev) {
        prev->UpdateLink(file, kIndexNext, sr3->FilePosition());
      }
    }
  }

  auto bytes = update ? IBlock::Update(file) : IBlock::Write(file);
  if (!update) {
    bytes += WriteNumber(file, record_id_);
    bytes += WriteNumber(file, nof_channels_);
    bytes += WriteNumber(file, size_of_data_record_);
    bytes += WriteNumber(file, nof_records_);
    bytes += WriteNumber(file, static_cast<uint32_t>(sr_link));
  } else {
      // Only nof_records and SR blocks may be added
    bytes += StepFilePosition(file, sizeof(record_id_) + sizeof(nof_channels_) + sizeof(size_of_data_record_));
    bytes += WriteNumber(file, nof_records_);
    // Check if any SR blocks been added. This typical happens after a measurement.
    if (sr_link > 0) {
      bytes += WriteNumber(file, static_cast<uint32_t>(sr_link));
    } else {
      bytes += StepFilePosition(file, sizeof(uint32_t));
    }
  }

  for (size_t cn_index = 0; cn_index < cn_list_.size(); ++cn_index) {
    auto& cn3 = cn_list_[cn_index];
    if (!cn3 || cn3->FilePosition() > 0) {
      continue;
    }
    cn3->Write(file);
    if (cn_index == 0) {
      UpdateLink(file, kIndexCn, cn3->FilePosition());
    } else {
      auto& prev = cn_list_[cn_index - 1];
      if (prev) {
        prev->UpdateLink(file, kIndexNext, cn3->FilePosition());
      }
    }
  }

  if (!comment_.empty() && Link(kIndexTx) <= 0) {
    Tx3Block tx(comment_);
    tx.Init(*this);
    tx.Write(file);
    UpdateLink(file, kIndexTx, tx.FilePosition());
  }
  return bytes;
}

void Cg3Block::ReadCnList(std::FILE *file) {
  if (cn_list_.empty() && Link(kIndexCn) > 0) {
    for (auto link = Link(kIndexCn); link > 0; /* No ++ here*/) {
      auto cn = std::make_unique<Cn3Block>();
      cn->Init(*this);
      SetFilePosition(file, link);
      cn->Read(file);
      link = cn->Link(kIndexNext);
      cn_list_.push_back(std::move(cn));
    }
  }
}

void Cg3Block::ReadSrList(std::FILE *file) {
  if (sr_list_.empty() && Link(kIndexSr) > 0) {
    for (auto link = Link(kIndexSr); link > 0; /* No ++ here*/) {
      auto sr = std::make_unique<Sr3Block>();
      sr->Init(*this);
      SetFilePosition(file, link);
      sr->Read(file);
      link = sr->Link(kIndexNext);
      sr_list_.push_back(std::move(sr));
    }
  }
}

const IBlock *Cg3Block::Find(int64_t index) const {
  for (const auto& p : cn_list_) {
    if (!p) {
      continue;
    }
    const auto* pp = p->Find(index);
    if (pp != nullptr) {
      return pp;
    }
  }
  return IBlock::Find(index);
}

std::string Cg3Block::Comment() const {
  return comment_;
}

void Cg3Block::AddCn3(std::unique_ptr<Cn3Block> &cn3) {
  cn_list_.push_back(std::move(cn3));
  nof_channels_ = static_cast<uint16_t>(cn_list_.size());
}

void Cg3Block::PrepareForWriting() {
  nof_channels_ = static_cast<uint16_t>(cn_list_.size());
  size_of_data_record_ = 0;
  for (auto& cn3 : cn_list_) {
    cn3->ByteOffset(size_of_data_record_);
    size_of_data_record_ += cn3->DataBytes();
  }
  sample_buffer_.resize(size_of_data_record_);
}

size_t Cg3Block::ReadDataRecord(std::FILE *file, const IDataGroup& notifier) const {
  size_t count = 0;

  // Normal fixed length records
  size_t record_size = size_of_data_record_;
  std::vector<uint8_t> record(record_size,0);
  count = std::fread(record.data(), 1, record.size(), file);
  size_t sample = Sample();
  if (sample < NofSamples()) {
    notifier.NotifySampleObservers(sample,RecordId(), record);
    IncrementSample();
  }

  return count;
}


}
