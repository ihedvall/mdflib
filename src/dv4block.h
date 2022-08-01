/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdio>
#include "datablock.h"
namespace mdf::detail {

class Dv4Block : public DataBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  [[nodiscard]] size_t DataSize() const override;
};

} // namespace
