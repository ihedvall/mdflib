/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "datalistblock.h"
#include "dt4block.h"
#include "dz4block.h"
#include "dl4block.h"
#include "hl4block.h"
#include "sr4block.h"
#include "rd4block.h"
#include "sd4block.h"

namespace {
  constexpr size_t kIndexNext = 0;
}

namespace mdf::detail {

void DataListBlock::ReadBlockList(std::FILE *file, size_t data_index) {
  if (block_list_.empty() && Link(data_index) > 0) {
    SetFilePosition(file, Link(data_index));
    std::string block_type = ReadBlockType(file);

    SetFilePosition(file, Link(data_index));
    if (block_type == "DT") {
      auto dt = std::make_unique<Dt4Block>();
      dt->Init(*this);
      dt->Read(file);
      block_list_.emplace_back(std::move(dt));
    } else if (block_type == "DZ") {
      auto dz = std::make_unique<Dz4Block>();
      dz->Init(*this);
      dz->Read(file);
      block_list_.emplace_back(std::move(dz));
    } else if (block_type == "DL") {
      for (auto link = Link(data_index); link > 0; /* no increment here*/) {
        SetFilePosition(file, link);
        auto dl = std::make_unique<Dl4Block>();
        dl->Init(*this);
        dl->Read(file);
        link = dl->Link(kIndexNext);
        block_list_.emplace_back(std::move(dl));
      }
    } else if (block_type == "HL") {
      auto hl = std::make_unique<Hl4Block>();
      hl->Init(*this);
      hl->Read(file);
      block_list_.emplace_back(std::move(hl));
    } else if (block_type == "SR") {
      for (auto link = Link(data_index); link > 0; /* no increment here*/) {
        SetFilePosition(file, link);
        auto sr = std::make_unique<Sr4Block>();
        sr->Init(*this);
        sr->Read(file);
        link = sr->Link(kIndexNext);
        block_list_.emplace_back(std::move(sr));
      }
    } else if (block_type == "RD") {
      auto rd = std::make_unique<Rd4Block>();
      rd->Init(*this);
      rd->Read(file);
      block_list_.emplace_back(std::move(rd));
    } else if (block_type == "SD") {
      auto sd = std::make_unique<Sd4Block>();
      sd->Init(*this);
      sd->Read(file);
      block_list_.emplace_back(std::move(sd));
    }
  }
}

  void DataListBlock::ReadLinkList(std::FILE *file, size_t data_index, uint32_t nof_link) {
    if (block_list_.empty()) {
      for (uint32_t ii = 0; ii < nof_link; ++ii) {
        auto link = Link(data_index + ii);
        if (link <= 0) {
          continue;
        }
        SetFilePosition(file, link);
        std::string block_type = ReadBlockType(file);

        SetFilePosition(file, link);
        if (block_type == "DT") {
          auto dt = std::make_unique<Dt4Block>();
          dt->Init(*this);
          dt->Read(file);
          block_list_.emplace_back(std::move(dt));
        } else if (block_type == "DZ") {
          auto dz = std::make_unique<Dz4Block>();
          dz->Init(*this);
          dz->Read(file);
          block_list_.emplace_back(std::move(dz));
        } else if (block_type == "RD") {
          auto rd = std::make_unique<Rd4Block>();
          rd->Init(*this);
          rd->Read(file);
          block_list_.emplace_back(std::move(rd));
        } else if (block_type == "SD") {
          auto sd = std::make_unique<Sd4Block>();
          sd->Init(*this);
          sd->Read(file);
          block_list_.emplace_back(std::move(sd));
         }
      }
    }
  }

const IBlock *DataListBlock::Find(fpos_t index) const {
  for (const auto& p : block_list_) {
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

size_t DataListBlock::DataSize() const {
  size_t count = 0;
  for (const auto& p : block_list_) {
    if (!p) {
      continue;
    }
    const auto* list = dynamic_cast<const DataListBlock*>(p.get());
    const auto* block = dynamic_cast<const DataBlock*>(p.get());
    if (list != nullptr) {
      count += list->DataSize();
    } else if (block != nullptr) {
      count += block->DataSize();
    }
  }
  return count;
}

} // namespace mdf::detail