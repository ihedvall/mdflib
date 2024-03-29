/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <vector>

#include "datablock.h"

namespace mdf::detail {

class Dt3Block : public DataBlock {
 public:
  void Init(const MdfBlock &id_block) override;
  void GetBlockProperty(BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
  [[nodiscard]] size_t DataSize() const override;

};

}  // namespace mdf::detail
