/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelarray.h
 * \brief Defines a channel array (CA) block. Its functionality is described
 * in the MDF 4 documentation.
 */
#pragma once
#include <cstdint>
#include <vector>
#include "mdf/iblock.h"

namespace mdf {

class IChannel;
class IChannelConversion;
class IChannelGroup;
class IDataGroup;

/** \brief Type of array.
 *
 */
enum class ArrayType : uint8_t {
  Array = 0, ///< Simple array without attributes
  ScalingAxis = 1, ///< Scaling axis.
  LookUp = 2, ///< Lookup array.
  IntervalAxis = 3, ///< Interval axis.
  ClassificationResult = 4 ///< Classification result.
};

/** \brief Type of storage. */
enum class ArrayStorage : uint8_t {
  CnTemplate = 0, ///< Channel template.
  CgTemplate = 1, ///< Channel group template.
  DgTemplate = 2  ///< Data group template.
};

/** \brief Channel array (CA) block flags. */
namespace CaFlag {
constexpr uint32_t DynamicSize = 0x0001; ///< Dynamic size
constexpr uint32_t InputQuantity = 0x0002; ///< Input quantity.
constexpr uint32_t OutputQuantity = 0x0004; ///< Output quantity.
constexpr uint32_t ComparisonQuantity = 0x0008; ///< Comparison quantity.
constexpr uint32_t Axis = 0x0010; ///< Axis
constexpr uint32_t FixedAxis = 0x0020; ///< Fixed axis.
constexpr uint32_t InverseLayout = 0x0040; ///< Inverse layout.
constexpr uint32_t LeftOpenInterval = 0x0080; ///< Left-over interval.
constexpr uint32_t StandardAxis = 0x0100; ///< Standard axis.
}  // namespace CaFlag

/** \brief Structure that defines the channel array (CA) blocks references.*/
struct  CaTripleReference {
  const mdf::IDataGroup* DataGroup = nullptr; ///< Pointer to the data group (DG) block
  const mdf::IChannelGroup* ChannelGroup = nullptr; ///< Pointer to the channel group (CG) block.
  const mdf::IChannel* Channel = nullptr; ///< Pointer to the channel (CN) block.
};


/** \brief Channel array */
class IChannelArray : public IBlock {
 public:

  virtual void Type(ArrayType type) = 0; ///< Type.
  [[nodiscard]] virtual ArrayType Type() const = 0; ///< Type of array

  virtual void Storage(ArrayStorage storage) = 0; ///< Storage type
  [[nodiscard]] virtual ArrayStorage Storage() const = 0; ///< Storage type

  virtual void Flags(uint32_t flags) = 0; ///< Flags
  [[nodiscard]] virtual uint32_t Flags() const = 0; ///< Flags

  [[nodiscard]] virtual uint16_t Dimensions() const = 0; ///< Number of dimensions
  virtual void DimensionSize(uint16_t dimension, uint64_t dimension_size ) = 0; ///< Size for a dimension
  [[nodiscard]] virtual uint64_t DimensionSize(uint16_t dimension) const = 0; ///< Size for a dimension

  [[nodiscard]] virtual uint64_t AxisValues() const = 0; ///< Number of axis values
  virtual void AxisValue(uint64_t index, double value ) = 0; ///< Setting an axis value
  [[nodiscard]] virtual double AxisValue(uint64_t index) const = 0; ///< Getting an axis value

  [[nodiscard]] virtual uint64_t CycleCounts() const = 0; ///< Number of cycle counts
  virtual void CycleCountOffset(uint64_t cycle_count, uint64_t offset ) = 0; ///< Setting an offset value
  [[nodiscard]] virtual uint64_t CycleCountOffset(uint64_t cycle_count) const = 0; ///< Getting an axis value

  /** \brief Returns the number of values in the array.
   *
   * @return Number of values in the array
   */
  [[nodiscard]] uint64_t NofArrayValues() const;

  /** \brief Returns the array dimensions as string ("N[2][3]")
   *
   * @return Array dimension as a string.
   */
  [[nodiscard]] std::string DimensionAsString() const;

  /** \brief Converts the array link indexes into pointers.
   *
   * This function must be called before getting any of the
   * link lists on the array object. It must be called later after the
   * entire file been read as the link reference blocks that are read
   * after this block.
   */
  virtual void CreateLinkLists() = 0;

  /** \brief Returns a list of data kink indexes. */
  [[nodiscard]] const std::vector<int64_t>& DataLinks() const {
    return data_links_;
  }
  /** \brief Returns the dynamic size list. */
  [[nodiscard]] const std::vector<CaTripleReference>& DynamicSizeList() const {
    return dynamic_size_list_;
  }
  /** \brief Returns the input quantity list. */
  [[nodiscard]] const std::vector<CaTripleReference>& InputQuantityList() const {
    return input_quantity_list_;
  }
  /** \brief Returns the output quantity list. */
  [[nodiscard]] const std::vector<CaTripleReference>& OutputQuantityList() const {
    return output_quantity_list_;
  }
  /** \brief Returns the comparison quantity list. */
  [[nodiscard]] const std::vector<CaTripleReference>& ComparisonQuantityList() const {
    return comparison_quantity_list_;
  }
  /** \brief Returns the axis conversion list. */
  [[nodiscard]] const std::vector<const IChannelConversion*>& AxisConversionList() const {
    return axis_conversion_list_;
  }
  /** \brief Returns the axis list. */
  [[nodiscard]] const std::vector<CaTripleReference>& AxisList() const {
    return axis_list_;
  }

 protected:
  std::vector<int64_t> data_links_; ///< List of index to data blocks.
  std::vector<CaTripleReference> dynamic_size_list_; ///< Dynamic size list.
  std::vector<CaTripleReference> input_quantity_list_; ///< Input quantity list.
  std::vector<CaTripleReference> output_quantity_list_; ///< Output quantity list.
  std::vector<CaTripleReference> comparison_quantity_list_; ///< Comparison quantity list.
  std::vector<const IChannelConversion*> axis_conversion_list_; ///< Axis_conversion list.
  std::vector<CaTripleReference> axis_list_; ///< Axis list.
};

}  // namespace mdf
