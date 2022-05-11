/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
#include "mdf4timestamp.h"
namespace mdf::detail {
class Fh4Block : public IBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
 private:
  Mdf4Timestamp timestamp_;
};
}





