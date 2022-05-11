/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <algorithm>
#include <ranges>
#include <cuchar>
#include "cg4block.h"


namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCn = 1;
constexpr size_t kIndexName = 2;
constexpr size_t kIndexSi = 3;
constexpr size_t kIndexSr = 4;
constexpr size_t kIndexMd = 5;


std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x0001) {
    s << "VLSD";
  }
  if (flag & 0x0002) {
    s << (s.str().empty() ? "Bus Event" : ",Bus Event");
  }
  if (flag & 0x0004) {
    s << (s.str().empty() ? "Plain" : ",Plain");
  }
  return s.str();
}

} // end namespace

namespace mdf::detail {

int64_t Cg4Block::Index() const {
  return FilePosition();
}

void Cg4Block::Name(const std::string &name) {
  acquisition_name_ = name;
}

std::string Cg4Block::Name() const {
  return acquisition_name_;
}

void Cg4Block::Description(const std::string &description) {
  md_comment_ = std::make_unique<Md4Block>(description);
}

std::string Cg4Block::Description() const {
  return MdText();
}

uint64_t Cg4Block::NofSamples() const {
  return nof_samples_;
}

void Cg4Block::NofSamples(uint64_t nof_samples) {
  nof_samples_ = nof_samples;
}
uint64_t Cg4Block::RecordId() const {
  return record_id_;
}
const IChannel *Cg4Block::GetXChannel(const IChannel &reference) const {
  const auto* cn4 = dynamic_cast<const Cn4Block*>(&reference);
  if (cn4 == nullptr) {
    return nullptr;
  }

  // First check if the channel have a dedicated X channel reference
  auto x_axis_list = cn4->XAxisLinkList();
  // As we are returning a channel pointer, we must assume that it belongs to this group
  if (x_axis_list.size() == 3 && x_axis_list[1] == Index() && x_axis_list[2]) {
    const auto channel_index = x_axis_list[2];
    auto find = std::ranges::find_if(cn_list_,[&] (const auto& p)  {
      if (!p) {
        return false;
      }
      return p->Index() == channel_index;
    });

    if (find != cn_list_.cend()) {
      return find->get();
    }
  }


  // Search for the master channel in the group
  auto master = std::ranges::find_if(cn_list_, [&] (const auto& x) {
    if (!x) {
      return false;
    }
    return x->Type() == ChannelType::Master || x->Type() == ChannelType::VirtualMaster;
  });

  return master != cn_list_.cend() ? master->get() : nullptr;
}

void Cg4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CG", ToHexString(Link(kIndexNext)), "Link to next channel group", BlockItemType::LinkItem );
  dest.emplace_back("First CN", ToHexString(Link(kIndexCn)), "Link to first channel",BlockItemType::LinkItem );
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), "Link to group name", BlockItemType::LinkItem );
  dest.emplace_back("Source SI", ToHexString(Link(kIndexSi)), "Link to source information", BlockItemType::LinkItem );
  dest.emplace_back("Reduction SR", ToHexString(Link(kIndexSr)), "Link to first sample reduction", BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);

  if (Link(kIndexName) > 0 ) {
    dest.emplace_back("Name", acquisition_name_);
  }
  dest.emplace_back("Nof Channels", std::to_string(cn_list_.size()));
  dest.emplace_back("Nof SR", std::to_string(sr_list_.size()));
  dest.emplace_back("Record ID", std::to_string(record_id_));
  dest.emplace_back("Nof Samples", std::to_string(nof_samples_));
  dest.emplace_back("Flags", MakeFlagString(flags_));

  std::mbstate_t state{};
  char utf8[MB_LEN_MAX]{};
  std::c16rtomb(utf8, path_separator_, &state);
  dest.emplace_back("Path Separator", utf8);

  dest.emplace_back("Data Bytes", std::to_string(nof_data_bytes_));
  dest.emplace_back("Invalid Bytes", std::to_string(nof_invalid_bytes_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Cg4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, record_id_);
  bytes += ReadNumber(file, nof_samples_);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, path_separator_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 4);
  bytes += ReadNumber(file, nof_data_bytes_);
  bytes += ReadNumber(file, nof_invalid_bytes_);

  acquisition_name_ = ReadTx4(file, kIndexName);
  if (Link(kIndexSi) > 0) {
    SetFilePosition(file, Link(kIndexSi));
    si_block_ = std::make_unique<Si4Block>();
    si_block_->Init(*this);
    si_block_->Read(file);
  }
  ReadMdComment(file,kIndexMd);
  return bytes;
}

void Cg4Block::ReadCnList(std::FILE *file) {
  if (cn_list_.empty() && Link(kIndexCn) > 0) {
    for (auto link = Link(kIndexCn); link > 0; /* No ++ here*/) {
      std::unique_ptr<Cn4Block> cn = std::make_unique<Cn4Block>();
      cn->Init(*this);
      SetFilePosition(file, link);
      cn->Read(file);
      link = cn->Link(kIndexNext);
      cn_list_.push_back(std::move(cn));
    }
  }
}

void Cg4Block::ReadSrList(std::FILE *file) {
  if (sr_list_.empty() && Link(kIndexSr) > 0) {
    for (auto link = Link(kIndexSr); link > 0; /* No ++ here*/) {
      std::unique_ptr<Sr4Block> sr = std::make_unique<Sr4Block>();
      sr->Init(*this);
      SetFilePosition(file, link);
      sr->Read(file);
      link = sr->Link(kIndexNext);
      sr_list_.push_back(std::move(sr));
    }
  }
}

const IBlock *Cg4Block::Find(fpos_t index) const {
  if (si_block_) {
    const auto* p = si_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (const auto& cn : cn_list_) {
    if (!cn) {
      continue;
    }
    const auto* p = cn->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (const auto& sr : sr_list_) {
    if (!sr) {
      continue;
    }
    const auto* p = sr->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  return IBlock::Find(index);
}

size_t Cg4Block::ReadDataRecord(std::FILE *file, const IDataGroup& notifier) const {
  size_t count = 0;
  if (flags_ & Cg4Flags::Vlsd) {
    // This is normally used for string and the CG block only include one signal
    uint32_t length = 0;
    count += ReadNumber(file, length);
    std::vector<uint8_t> record(length,0);
    if (length > 0) {
      count += std::fread(record.data(), 1, length, file);
    }
    size_t sample = Sample();
    if (sample < NofSamples()) {
      notifier.NotifySampleObservers(sample,RecordId(), record);
      IncrementSample();
    }
  } else {
    // Normal fixed length records
    size_t record_size = nof_data_bytes_ + nof_invalid_bytes_;
    std::vector<uint8_t> record(record_size,0);
    count = std::fread(record.data(), 1, record.size(), file);
    size_t sample = Sample();
    if (sample < NofSamples()) {
      notifier.NotifySampleObservers(sample,RecordId(), record);
      IncrementSample();
    }
  }
  return count;
}

std::vector<IChannel *> Cg4Block::Channels() const {
   std::vector<IChannel *> channel_list;
   std::ranges::for_each(cn_list_, [&] (const auto& cn3) {channel_list.push_back(cn3.get()); } );
   return channel_list;
}


}