/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <vector>
#include "datablock.h"


namespace mdf::detail {
class Dg3Block;
class Dt3Block : public DataBlock {
 public:
  void Init(const IBlock &id_block) override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
  size_t DataSize() const override;
 private:
  const Dg3Block* dg_block_ = nullptr; ///< Pointer to the for calculating data size
};

}

