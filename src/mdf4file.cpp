/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf4file.h"
#include <ranges>
#include "mdf/mdflogstream.h"

namespace mdf::detail {

Mdf4File::Mdf4File()
    : id_block_(std::make_unique<IdBlock>()),
      hd_block_(std::make_unique<Hd4Block>()) {
  hd_block_->Init(*id_block_);
}

Mdf4File::Mdf4File(std::unique_ptr<IdBlock> id_block)
    : id_block_(std::move(id_block)) {}

IHeader *Mdf4File::Header() const { return hd_block_.get(); }

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

  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  if (!finalized_done_ && id_block_ &&
      !id_block_->IsFinalized(standard_flags, custom_flags)) {
    finalized_done_ = true;
    // Try to finalize the last DG block.
    const auto finalize = FinalizeFile(file);
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

bool Mdf4File::IsMdf4() const { return true; }

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

const MdfBlock *Mdf4File::Find(int64_t id) const {
  if (id_block_) {
    const auto *p = id_block_->Find(id);
    if (p != nullptr) {
      return p;
    }
  }
  if (hd_block_) {
    const auto *p = hd_block_->Find(id);
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
  const auto &at4_list = hd_block_->At4();
  for (const auto &at4 : at4_list) {
    dest.emplace_back(at4.get());
  }
}
void Mdf4File::DataGroups(DataGroupList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto &dg4_list = hd_block_->Dg4();
  for (const auto &dg4 : dg4_list) {
    dest.emplace_back(dg4.get());
  }
}

std::string Mdf4File::Version() const {
  return !id_block_ ? "" : id_block_->VersionString();
}

void Mdf4File::MinorVersion(int minor) {
  if (id_block_) {
    id_block_->MinorVersion(minor);
  }
}

void Mdf4File::ProgramId(const std::string &program_id) {
  if (id_block_) {
    id_block_->ProgramId(program_id);
  }
}
std::string Mdf4File::ProgramId() const {
  return !id_block_ ? "" : id_block_->ProgramId();
}

IAttachment *Mdf4File::CreateAttachment() {
  IAttachment *attachment = nullptr;
  if (hd_block_) {
    auto at4 = std::make_unique<At4Block>();
    at4->Init(*hd_block_);
    hd_block_->AddAt4(at4);
    const auto &at_list = hd_block_->At4();
    attachment = at_list.empty() ? nullptr : at_list.back().get();
  }
  return attachment;
}

IDataGroup *Mdf4File::CreateDataGroup() {
  IDataGroup *dg = nullptr;
  if (hd_block_) {
    auto dg4 = std::make_unique<Dg4Block>();
    dg4->Init(*hd_block_);
    hd_block_->AddDg4(dg4);
    dg = hd_block_->LastDataGroup();
  }
  return dg;
}

bool Mdf4File::Write(std::FILE *file) {
  if (file == nullptr) {
    MDF_ERROR() << "File pointer is null. Invalid use of function.";
    return false;
  }
  if (!id_block_ || !hd_block_) {
    MDF_ERROR() << "No ID or HD block defined. Invalid use of function.";
    return false;
  }

  try {
    id_block_->Write(file);
    hd_block_->Write(file);
  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to write the MDF4 file. Error: " << err.what();
    return false;
  }
  return true;
}

void Mdf4File::IsFinalized(bool finalized, std::FILE *file,
                           uint16_t standard_flags, uint16_t custom_flags) {
  if (id_block_) {
    id_block_->IsFinalized(finalized, file, standard_flags, custom_flags);
  }
}

bool Mdf4File::IsFinalized(uint16_t &standard_flags,
                           uint16_t &custom_flags) const {
  bool finalized = true;
  if (id_block_) {
    finalized = id_block_->IsFinalized(standard_flags, custom_flags);
  } else {
    standard_flags = 0;
    custom_flags = 0;
  }
  return finalized;
}

bool Mdf4File::FinalizeFile(std::FILE* file) {
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  const auto finalized = IsFinalized(standard_flags, custom_flags);
  if (finalized || standard_flags == 0 || !hd_block_) {
    MDF_DEBUG()
        << "The files standard flags are OK. Cannot finalize the file. File: "
        << Name();
    return false;
  }
  ReadEverythingButData(file);

  // 1. Update DL block
  // 2. Update DT  block
  // 3. Update RD block
  // 4. Update CG (CA) block
  const bool update_cg_blocks = (standard_flags & 0x01) != 0;
  const bool update_sr_blocks = (standard_flags & 0x02) != 0;
  const bool update_dt_blocks = (standard_flags & 0x04) != 0;
  const bool update_rd_blocks = (standard_flags & 0x08) != 0;
  const bool update_dl_blocks = (standard_flags & 0x10) != 0;
  const bool update_vlsd_bytes = (standard_flags & 0x20) != 0;
  const bool update_vlsd_offset = (standard_flags & 0x40) != 0;
  bool updated = true;
  if (update_dl_blocks) {
    updated = false;
  }
  if (update_dt_blocks && !hd_block_->UpdateDtBlocks(file)) {
    updated = false;
  }
  if (update_rd_blocks) {
    updated = false;
  }
  if ((update_cg_blocks || update_vlsd_bytes || update_vlsd_offset ) &&
      !hd_block_->UpdateCgAndVlsdBlocks(file, update_cg_blocks,
                                update_vlsd_bytes || update_vlsd_offset)) {
    updated = false;
  }
  return updated;
}

const IDataGroup *Mdf4File::FindParentDataGroup(const IChannel &channel) const {
  const auto channel_index = channel.Index();
  if (!hd_block_ || channel_index <= 0) {
    return nullptr;
  }
  const auto &dg_list = hd_block_->Dg4();
  const auto itr = std::ranges::find_if(dg_list, [&](const auto &dg_block) {
    return dg_block && dg_block->Find(channel_index) != nullptr;
    });

  return itr != dg_list.cend() ? itr->get() : nullptr;
}

}  // namespace mdf::detail