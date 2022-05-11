/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ch4block.h"
namespace {

constexpr size_t kIndexCh = 1;
constexpr size_t kIndexTx = 2;
constexpr size_t kIndexMd = 3;
constexpr size_t kIndexElement = 4;
constexpr size_t kIndexNext = 0;
}

namespace mdf::detail {
void Ch4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "",BlockItemType::HeaderItem);
  dest.emplace_back("Next CH", ToHexString(Link(kIndexNext)), "", BlockItemType::LinkItem);
  dest.emplace_back("First CH", ToHexString(Link(kIndexNext)), "", BlockItemType::LinkItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexNext)), "", BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "", BlockItemType::LinkItem);
  for (uint32_t ii = 0; ii < nof_elements_; ++ii) {
    size_t index = kIndexElement + (3 * ii);
    dest.emplace_back("Reference DG", ToHexString(Link(index)), "", BlockItemType::LinkItem);
    dest.emplace_back("Reference CG", ToHexString(Link(index + 1)), "", BlockItemType::LinkItem);
    dest.emplace_back("Reference CN", ToHexString(Link(index + 2)), "", BlockItemType::LinkItem);
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Nof Elements", std::to_string(nof_elements_));
  dest.emplace_back("Type", std::to_string(type_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Ch4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, nof_elements_);
  bytes += ReadNumber(file, type_);
  name_ = ReadTx4(file,kIndexTx);
  ReadMdComment(file,kIndexMd);
  if (ch_list_.empty() && Link(kIndexCh) > 0) {
    for (auto link = Link(kIndexCh); link > 0; /* No ++ here*/) {
      auto ch = std::make_unique<Ch4Block>();
      ch->Init(*this);
      SetFilePosition(file, link);
      ch->Read(file);
      link = ch->Link(kIndexNext);
      ch_list_.emplace_back(std::move(ch));
    }
  }
  ref_list.clear();
  for (uint32_t ii = 0; ii < nof_elements_; ++ii) {
    size_t index = kIndexElement + (3 * ii);
    ref_list.push_back(Link(index));
    ref_list.push_back(Link(index + 1));
    ref_list.push_back(Link(index + 2));
  }
  return bytes;
}

const IBlock *Ch4Block::Find(fpos_t index) const {
  for (const auto& ch : ch_list_) {
    if (!ch) {
      continue;
    }
    const auto* p = ch->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return IBlock::Find(index);
}

}