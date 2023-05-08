/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>
#include <vector>

#include "mdfblock.h"

namespace mdf::detail {
class DataListBlock : public MdfBlock {
 public:
  using BlockList = std::vector<std::unique_ptr<MdfBlock>>;
  BlockList& DataBlockList() { return block_list_; }
  [[nodiscard]] const BlockList& DataBlockList() const { return block_list_; }
  [[nodiscard]] virtual size_t DataSize() const;
  [[nodiscard]] const MdfBlock* Find(int64_t index) const override;
  void ReadBlockList(std::FILE* file, size_t data_index);
  void WriteBlockList(std::FILE* file, size_t data_index);

  void ReadLinkList(std::FILE* file, size_t data_index, uint32_t nof_link);
  void ClearData();
 protected:
  BlockList block_list_;
};
}  // namespace mdf::detail
