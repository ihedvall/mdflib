/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datalistblock.h"
namespace mdf::detail {
class Hl4Block : public DataListBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
 private:
  uint16_t flags_ = 0;
  uint8_t type_ = 0;
  /* 5 byte reserved */
};
}
