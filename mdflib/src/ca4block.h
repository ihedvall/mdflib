/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/ichannelarray.h"
#include "mdfblock.h"
namespace mdf::detail {

class Ca4Block : public MdfBlock, public IChannelArray {
 public:
  using Cx4List = std::vector<std::unique_ptr<MdfBlock>>;
  Ca4Block();

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Type(ArrayType type) override;
  [[nodiscard]] ArrayType Type() const override;
  [[nodiscard]] std::string TypeAsString() const;

  void Storage(ArrayStorage storage) override;
  [[nodiscard]] ArrayStorage Storage() const override;
  [[nodiscard]] std::string StorageAsString() const;

  void Flags(uint32_t flags) override;
  [[nodiscard]] uint32_t Flags() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;

  [[nodiscard]] uint16_t Dimensions() const override; ///< Number of dimensions
  void DimensionSize(uint16_t dimension, uint64_t dimension_size ) override; ///< Size for a dimension
  [[nodiscard]] uint64_t DimensionSize(uint16_t dimension) const override; ///< Size for a dimension

  [[nodiscard]] uint64_t AxisValues() const override; ///< Number of axis
  void AxisValue(uint64_t index, double value ) override; ///< Setting an axis value
  [[nodiscard]] double AxisValue(uint64_t index) const override; ///< Getting an axis value

  [[nodiscard]] uint64_t CycleCounts() const override; ///< Number of cycle counts
  void CycleCountOffset(uint64_t cycle_count, uint64_t offset ) override; ///< Setting an offset value
  [[nodiscard]] uint64_t CycleCountOffset(uint64_t cycle_count) const override; ///< Getting an axis value

  [[nodiscard]] const MdfBlock *Find(int64_t index) const override;
  [[nodiscard]] const Cx4List& Cx4() const { return composition_list_; }

  size_t Read(std::FILE* file) override;

 private:
  uint8_t type_ = 0;
  uint8_t storage_ = 0;
  uint16_t dimensions_ = 0; // Number of dimensions (N)
  uint32_t flags_ = 0;
  int32_t byte_offset_base_ = 0;
  uint32_t invalid_bit_pos_base_ = 0;

  /** \brief Array of Array or composition _list */
  Cx4List composition_list_;

  // Store number of items in each dimension (d).
  std::vector<uint64_t> dim_size_list_; ///< Dimension size for each dimension
  // Axis values
  std::vector<double> axis_value_list_;
  std::vector<uint64_t> cycle_count_list_;
  [[nodiscard]] uint64_t NumberOfAxisValues() const;

  void CreateLinkLists() override;



};
}  // namespace mdf::detail
