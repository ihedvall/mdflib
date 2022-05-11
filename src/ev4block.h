/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"

namespace mdf::detail {
class Ev4Block : public IBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
 private:
  uint8_t type_ = 0;
  uint8_t sync_type_ = 0;
  uint8_t range_type_ = 0;
  uint8_t cause_ = 0;
  uint8_t flags_ = 0;
  /* 1 byte reserved */
  uint32_t length_m_ = 0; ///< Number of scope events
  uint16_t length_n_ = 0; ///< Number of attachments
  uint16_t creator_index_ = 0;
  int64_t sync_base_value_ = 0;
  double sync_factor_ = 0;

  std::string name_;
};
}




