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
#include "mdf/iblock.h"

namespace mdf {

/** \brief Type of array.
 *
 */
enum class ArrayType : uint8_t {
  Array = 0, ///< Array
  ScalingAxis = 1, ///< Scaling axis.
  LookUp = 2, ///< Lookup array.
  IntervalAxis = 3, ///< Interval axis.
  ClassificationResult = 4 ///< Classification result.
};

/** \brief Type of storage.
 *
 */
enum class ArrayStorage : uint8_t {
  CnTemplate = 0, ///< Channel template.
  CgTemplate = 1, ///< Channel group template.
  DgTemplate = 2  ///< Data group template.
};

/** \brief Channel array flags..
 *
 */
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

/** \brief Channel array
 *
 */
class IChannelArray : public IBlock {
 public:

  virtual void Type(ArrayType type) = 0; ///< Type.
  [[nodiscard]] virtual ArrayType Type() const = 0; ///< Type

  virtual void Storage(ArrayStorage storage) = 0; ///< Storage
  [[nodiscard]] virtual ArrayStorage Storage() const = 0; ///< Storage

  virtual void Flags(uint32_t flags) = 0; ///< Flags
  [[nodiscard]] virtual uint32_t Flags() const = 0; ///< Flags
};

}  // namespace mdf
