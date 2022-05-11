/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <vector>
#include <algorithm>
#include <util/timestamp.h>
#include "hd3block.h"
#include "pr3block.h"

namespace {

constexpr size_t kIndexDg = 0;
constexpr size_t kIndexTx = 1;
constexpr size_t kIndexPr = 2;
constexpr size_t kIndexNext = 0;

}

namespace mdf::detail {

const Hd3Block::Dg3List& Hd3Block::Dg3() const {
  return dg_list_;
}

std::string Hd3Block::Comment() const {
  return comment_;
}

const IBlock *Hd3Block::Find(fpos_t index) const {
  if (pr_block_) {
    const auto* p = pr_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (const auto& dg : dg_list_) {
    if (!dg) {
      continue;
    }
    const auto* p = dg->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return IBlock::Find(index);
}

void Hd3Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("First DG", ToHexString(Link(kIndexDg)), "Link to first data group", BlockItemType::LinkItem );
  dest.emplace_back("Comment TX", ToHexString(Link(kIndexTx)), comment_, BlockItemType::LinkItem);
  if (pr_block_) {
    std::string pr_text = pr_block_->Text();
    if (pr_text.size() > 20) {
      pr_text.resize(20);
      pr_text += "..";
    }
    dest.emplace_back("Program PR", ToHexString(Link(kIndexPr)),
                      pr_text,
                      BlockItemType::LinkItem);
  } else {
    dest.emplace_back("Program PR", ToHexString(Link(kIndexPr)),
                      "Link to program data",
                      BlockItemType::LinkItem);
  }
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Number of DG Blocks", std::to_string(nof_dg_blocks_));
  dest.emplace_back("Date", date_);
  dest.emplace_back("Time", time_);
  dest.emplace_back("Author", author_);
  dest.emplace_back("Project", project_);
  dest.emplace_back("Organization", organisation_);
  dest.emplace_back("Time Stamp [ns]", std::to_string(local_timestamp_));
  dest.emplace_back("UTC Time Offset [h]", std::to_string(dst_offset_));
  switch (time_quality_) {
    case 0:
      dest.emplace_back("Time Source", "PC Time");
      break;

    case 10:
      dest.emplace_back("Time Source", "External");
      break;

    case 16:
      dest.emplace_back("Time Source", "NTP/PTP");
      break;

  }
  dest.emplace_back("Timer ID", timer_id_);
  dest.emplace_back("Comment", comment_);
}

size_t Hd3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadLinks3(file, 3);
  // The for loop handles earlier versions of the MDF file
  for (int ii = 3; bytes < block_size_; ++ii) {
    switch (ii) {
      case 3:bytes += ReadNumber(file, nof_dg_blocks_);
        break;
      case 4:bytes += ReadStr(file, date_, 10);
        break;
      case 5:bytes += ReadStr(file, time_, 8);
        break;
      case 6:bytes += ReadStr(file, author_, 32);
        break;
      case 7:bytes += ReadStr(file, organisation_, 32);
        break;
      case 8:bytes += ReadStr(file, project_, 32);
        break;
      case 9:bytes += ReadStr(file, subject_, 32);
        break;
      case 10:bytes += ReadNumber(file, local_timestamp_);
        break;
      case 11:bytes += ReadNumber(file, dst_offset_);
        break;
      case 12:bytes += ReadNumber(file, time_quality_);
        break;
      case 13:bytes += ReadStr(file, timer_id_, 32);
        break;
      default:std::string temp;
        bytes += ReadStr(file, temp, 1);
        break;
    }
  }
  comment_ = ReadTx3(file, kIndexTx);

  if (Link(kIndexPr) > 0) {
    pr_block_ = std::make_unique<Pr3Block>();
    pr_block_->Init(*this);
    SetFilePosition(file, Link(kIndexPr));
    pr_block_->Read(file);
  }
  return bytes;
}

size_t Hd3Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0; // Write or update the values inside the block
  if (!update) {
    block_type_ = "HD";
    block_size_ = (2 + 2) + (3*4) + 2 + 10 + 8 + 4*32 + 3*2 + 32;
    link_list_.resize(3,0);
  }

  auto bytes = update ? IBlock::Update(file) : IBlock::Write(file);

  nof_dg_blocks_ = static_cast<uint16_t>(dg_list_.size());

  bytes += WriteNumber(file, nof_dg_blocks_);
  bytes += WriteStr(file, date_, 10);
  bytes += WriteStr(file, time_, 8);
  bytes += WriteStr(file, author_, 32);
  bytes += WriteStr(file, organisation_, 32);
  bytes += WriteStr(file, project_, 32);
  bytes += WriteStr(file, subject_, 32);
  bytes += WriteNumber(file, local_timestamp_);
  bytes += WriteNumber(file, dst_offset_);
  bytes += WriteNumber(file, time_quality_);
  bytes += WriteStr(file, timer_id_, 32);

  if (!update) {
    UpdateBlockSize(file, bytes);
  }

  if (!comment_.empty() && Link(kIndexTx) <= 0) {
    Tx3Block tx(comment_);
    tx.Init(*this);
    tx.Write(file);
    UpdateLink(file, kIndexTx,tx.FilePosition());
  }

  if (pr_block_ && Link(kIndexPr) <= 0) {
    pr_block_->Init(*this);
    pr_block_->Write(file);
    UpdateLink(file, kIndexPr,pr_block_->FilePosition());
  }

  for (size_t index = 0; index < dg_list_.size(); ++index) {
    auto& dg3 = dg_list_[index];
    if (!dg3) {
      continue;
    }

    // Only last DG is updated
    const bool last_dg = index + 1 >= dg_list_.size(); // Only last DG is updated
    if (!last_dg && dg3->FilePosition() > 0) {
      continue;
    }

    dg3->Write(file);
    if (index == 0) {
      UpdateLink(file, kIndexDg, dg3->FilePosition());
    } else {
      auto& prev = dg_list_[index -1];
      if (prev) {
        prev->UpdateLink(file, kIndexNext, dg3->FilePosition());
      }
    }
  }
  return bytes;
}

void Hd3Block::ReadMeasurementInfo(std::FILE *file) {
  dg_list_.clear();
  for (auto link = Link(kIndexDg); link > 0; /* No ++ here*/) {
    auto dg = std::make_unique<Dg3Block>();
    dg->Init(*this);
    SetFilePosition(file, link);
    dg->Read(file);
    link = dg->Link(kIndexNext);
    dg_list_.emplace_back(std::move(dg));
  }
}

void Hd3Block::ReadEverythingButData(std::FILE *file) {
  // We assume that ReadMeasurementInfo have been called earlier
  for ( auto& dg : dg_list_) {
    if (!dg) {
      continue;
    }
    for (auto& cg : dg->Cg3()) {
      cg->ReadCnList(file);
      cg->ReadSrList(file);
    }
  }
}

int64_t Hd3Block::Index() const {
  return FilePosition();
}

void Hd3Block::Author(const std::string &author) {
  author_ = author;
}

std::string Hd3Block::Author() const {
  return author_;
}

void Hd3Block::Department(const std::string &department) {
  organisation_ = department;
}

std::string Hd3Block::Department() const {
  return organisation_;
}

void Hd3Block::Project(const std::string &name) {
  project_ = name;
}

std::string Hd3Block::Project() const {
  return project_;
}

void Hd3Block::Subject(const std::string &subject) {
  subject_ = subject;
}

std::string Hd3Block::Subject() const {
  return subject_;
}

void Hd3Block::Description(const std::string &description) {
  comment_ = description;
}

std::string Hd3Block::Description() const {
  return comment_;
}

void Hd3Block::StartTime(uint64_t ns_since_1970) {
  date_ = util::time::NanoSecToDDMMYYYY(ns_since_1970);
  time_ = util::time::NanoSecToHHMMSS(ns_since_1970);
  local_timestamp_ =  util::time::NanoSecToLocal(ns_since_1970);
  dst_offset_ = static_cast<int16_t>(util::time::TimeZoneOffset() / 3600);
  time_quality_ = 0;
  timer_id_ = "Local PC Reference Time";
}

uint64_t Hd3Block::StartTime() const {
  return local_timestamp_ - (util::time::TimeZoneOffset() * 1'000'000'000LL);
}

void Hd3Block::MetaData(const std::string &meta_data) {
  // Create a PR block with data
  pr_block_ = std::make_unique<Pr3Block>(meta_data);
  pr_block_->Init(*this);
}

std::string Hd3Block::MetaData() const {
  return !pr_block_ ? std::string() : pr_block_->Text();
}

IDataGroup *Hd3Block::LastDataGroup() const {
  return dg_list_.empty() ? nullptr : dg_list_.back().get();
}

std::vector<IDataGroup *> Hd3Block::DataGroups() const {
  std::vector<IDataGroup *> list;
  std::ranges::for_each(dg_list_, [&] (const auto& dg3) { list.push_back(dg3.get());  });
  return list;
}

void Hd3Block::AddDg3(std::unique_ptr<Dg3Block> &dg3) {
  dg3->Init(*this);
  dg_list_.push_back(std::move(dg3));
}



} // end namespace mdf::detail