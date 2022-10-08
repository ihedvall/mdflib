/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
#include "mdf/ichannelarray.h"
namespace mdf::detail {

class Ca4Block : public IBlock, public IChannelArray {
public:
  Ca4Block();

  [[nodiscard]] int64_t Index() const override;

  void Type(ArrayType type) override;
  [[nodiscard]] ArrayType Type() const override;

  void Storage(ArrayStorage storage) override;
  [[nodiscard]] ArrayStorage Storage() const override;

  void Flags(uint32_t flags) override;
  [[nodiscard]] uint32_t Flags() const override;

  void GetBlockProperty(BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;

private:
  uint8_t type_ = 0;
  uint8_t storage_ = 0;
  uint16_t dimension_ = 0;
  uint32_t flags_ = 0;
  int32_t byte_offset_base_ = 0;
  uint32_t invalid_bit_pos_base_ = 0;
  std::vector<uint64_t> dim_size_list_;
  std::vector<double> axis_value_list_;
  std::vector<uint16_t> cycle_count_list_;
};
} // namespace mdf::detail
