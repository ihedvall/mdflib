/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datalistblock.h"

namespace mdf::detail {
namespace Dl4Flags {
constexpr uint8_t EqualLength = 0x01;
}

class Dl4Block : public DataListBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
 private:
  uint8_t flags_ = 0;
  /* 3 byte reserved */
  uint32_t nof_blocks_ = 0;
  uint64_t equal_length_ = 0;

  std::vector<uint64_t> offset_list_;
};
}




