/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "fh4block.h"
namespace {
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexMd = 1;
}
namespace mdf::detail {

void Fh4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "",BlockItemType::HeaderItem);
  dest.emplace_back("Next FH", ToHexString(Link(kIndexNext)), "", BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "", BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("FH Info", "", "", BlockItemType::HeaderItem);
  timestamp_.GetBlockProperty(dest);
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Fh4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  timestamp_.Init(*this);
  bytes += timestamp_.Read(file);
  ReadMdComment(file, kIndexMd);
  return bytes;
}
}