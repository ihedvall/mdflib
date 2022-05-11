/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datalistblock.h"

namespace mdf::detail {
class Sr4Block : public DataListBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
 private:
  uint64_t nof_samples_ = 0;
  double interval_ = 0;
  uint8_t type_ = 0;
  uint8_t flags_ = 0;
  /* reserved 6 byte */
};
}


