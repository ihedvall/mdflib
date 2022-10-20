/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>

#include "datablock.h"
namespace mdf::detail {
class Dt4Block : public DataBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE* file) override;
  [[nodiscard]] size_t DataSize() const override;
  void UpdateDataSize(std::FILE* file);
};

}  // namespace mdf::detail
