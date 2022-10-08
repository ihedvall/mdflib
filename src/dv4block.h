/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "datablock.h"
#include <cstdio>
namespace mdf::detail {

class Dv4Block : public DataBlock {
public:
  void GetBlockProperty(BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;
  [[nodiscard]] size_t DataSize() const override;
};

} // namespace mdf::detail
