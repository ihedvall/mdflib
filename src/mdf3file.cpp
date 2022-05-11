/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <memory>
#include <stdexcept>
#include <cstdio>
#include "mdf3file.h"

using namespace util::log;

namespace mdf::detail {

Mdf3File::Mdf3File()
: id_block_(std::make_unique<IdBlock>()),
  hd_block_(std::make_unique<Hd3Block>())
{
  id_block_->SetDefaultMdf3Values();
  hd_block_->Init(*id_block_);
}

Mdf3File::Mdf3File(std::unique_ptr<IdBlock> id_block)
    : id_block_(std::move(id_block)) {
}

void Mdf3File::Attachments(AttachmentList &dest) const {
}

void Mdf3File::DataGroups(DataGroupList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto& dg_list = hd_block_->Dg3();
  for (const auto& dg : dg_list) {
    dest.emplace_back(dg.get());
  }
}

IHeader *Mdf3File::Header() const {
  return hd_block_.get();
}

void Mdf3File::ReadHeader(std::FILE *file) {
  if (!id_block_) {
    id_block_ = std::make_unique<IdBlock>();
  }
  if (id_block_->FilePosition() < 0) {
    SetFilePosition(file, 0);
    id_block_->Read(file);
  }

  if (!hd_block_) {
    hd_block_ = std::make_unique<Hd3Block>();
  }
  if (hd_block_->FilePosition() <= 0) {
    hd_block_->Init(*id_block_);
    SetFilePosition(file, 64);
    hd_block_->Read(file);
  }

}

void Mdf3File::ReadMeasurementInfo(std::FILE *file) {
  ReadHeader(file);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(file);
  }
}
void Mdf3File::ReadEverythingButData(std::FILE *file) {
  ReadHeader(file);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(file);
    hd_block_->ReadEverythingButData(file);
  }
}
bool Mdf3File::IsMdf4() const {
  return false;
}

const IdBlock &Mdf3File::Id() const {
  if (!id_block_) {
    throw std::domain_error("ID block not initialized yet");
  }
  return *id_block_;
}

const Hd3Block &Mdf3File::Hd() const {
  if (!hd_block_) {
    throw std::domain_error("HD3 block not initialized yet");
  }
  return *hd_block_;
}

const IBlock *Mdf3File::Find(fpos_t id) const {
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

IDataGroup *Mdf3File::CreateDataGroup() {
  IDataGroup* dg = nullptr;
  if (hd_block_) {
    auto dg3 = std::make_unique<Dg3Block>();
    dg3->Init(*hd_block_);
    dg = dg3.get();
    hd_block_->AddDg3(dg3);
  }
  return dg;
}

bool Mdf3File::Write(std::FILE* file) {
  if (file == nullptr) {
    LOG_ERROR() << "File pointer is null. Invalid use of function.";
    return false;
  }
  if (!id_block_ || !hd_block_) {
    LOG_ERROR() << "No ID or HD block defined. Invalid use of function.";
    return false;
  }

  id_block_->Write(file);
  hd_block_->Write(file);
  return true;
}

std::string Mdf3File::Version() const {
  return !id_block_ ? "" : id_block_->VersionString();
}

std::string Mdf3File::ProgramId() const {
  return !id_block_ ? "" : id_block_->ProgramId();
}

}
