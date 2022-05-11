/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <stdexcept>
#include "dt3block.h"
#include "dg3block.h"
namespace mdf::detail {

void Dt3Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Data Size [byte]", std::to_string(DataSize()));
}
void Dt3Block::Init(const IBlock &id_block) {
  dg_block_ = dynamic_cast<const Dg3Block*>(&id_block);
  IBlock::Init(id_block);
}

size_t Dt3Block::Read(std::FILE *file) {
  block_type_ = "DT";
  file_position_ =  GetFilePosition(file);
  data_position_ = file_position_;
  return 0;
}

size_t Dt3Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  SetLastFilePosition(file);
  block_type_ = "DT";
  file_position_ = GetFilePosition(file);
  data_position_ = file_position_;
  // ToDo: Fix write of data
  return 0;
}

size_t Dt3Block::DataSize() const {
  size_t bytes = 0;
  if (dg_block_ == nullptr) {
    return 0;
  }
  for (const auto& cg3 : dg_block_->Cg3()) {
    if (!cg3) {
      continue;
    }
    bytes += (dg_block_->NofRecordId() + cg3->RecordSize()) * cg3->NofSamples();
  }
  return bytes;
}


}