/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf4file.h"
namespace mdf::detail {
Mdf4File::Mdf4File(std::unique_ptr<IdBlock> id_block)
    : id_block_(std::move(id_block)) {

}

IHeader *Mdf4File::Header() const {
  return hd_block_.get();
}

void Mdf4File::ReadHeader(std::FILE *file) {
  if (!id_block_) {
    id_block_ = std::make_unique<IdBlock>();
    SetFilePosition(file, 0);
    id_block_->Read(file);
  }
  if (!hd_block_) {
    hd_block_ = std::make_unique<Hd4Block>();
    hd_block_->Init(*id_block_);
    SetFilePosition(file, 64);
    hd_block_->Read(file);
  }

}

void Mdf4File::ReadMeasurementInfo(std::FILE *file) {
  ReadHeader(file);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(file);
  }
}

void Mdf4File::ReadEverythingButData(std::FILE *file) {
  ReadHeader(file);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(file);
    hd_block_->ReadEverythingButData(file);
  }

}
bool Mdf4File::IsMdf4() const {
  return true;
}

const IdBlock &Mdf4File::Id() const {
  if (!id_block_) {
    throw std::domain_error("ID block not initialized yet");
  }
  return *id_block_;
}
const Hd4Block &Mdf4File::Hd() const {
  if (!hd_block_) {
    throw std::domain_error("HD4 block not initialized yet");
  }
  return *hd_block_;
}

const IBlock *Mdf4File::Find(fpos_t id) const {
  if (id_block_) {
    const auto* p = id_block_->Find(id);
    if (p != nullptr) {
      return p;
    }
  }
  if (hd_block_) {
    const auto* p = hd_block_->Find(id);
    if (p != nullptr) {
      return p;
    }
  }
  return nullptr;
}

void Mdf4File::Attachments(AttachmentList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto& at4_list = hd_block_->At4();
  for (const auto& at4 : at4_list) {
    dest.emplace_back(at4.get());
  }
}
void Mdf4File::DataGroups(DataGroupList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto& dg4_list = hd_block_->Dg4();
    for (const auto& dg4 : dg4_list) {
      dest.emplace_back(dg4.get());
    }
}

std::string Mdf4File::Version() const {
  return !id_block_ ? "" : id_block_->VersionString();
}

std::string Mdf4File::ProgramId() const {
  return !id_block_ ? "" : id_block_->ProgramId();
}

} // namespace mdf::detail