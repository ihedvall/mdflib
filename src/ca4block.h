/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
namespace mdf::detail {
namespace Ca4Flags {
constexpr uint32_t kDynamicSize = 0x01;
constexpr uint32_t kInputQuantity = 0x02;
constexpr uint32_t kOutputQuantity = 0x04;
constexpr uint32_t kComparisonQuantity = 0x08;
constexpr uint32_t kAxis = 0x10;
constexpr uint32_t kFixedAxis = 0x20;
constexpr uint32_t kInverseLayout = 0x40;
constexpr uint32_t kLeftOpenInterval = 0x80;
}

enum class Ca4Type : uint8_t {
  Array = 0,
  ScalingAxis = 1,
  LookUp = 2,
  IntervalAxis = 3,
  ClassificationResult = 4
};

enum class Ca4Storage : uint8_t {
  kCnTemplate = 0,
  kCgTemplate = 1,
  kDgTemplate = 2
};
class Ca4Block : public IBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
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
}





