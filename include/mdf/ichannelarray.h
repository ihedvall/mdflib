/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include "mdf/iblock.h"

namespace mdf {

enum class ArrayType : uint8_t {
  Array = 0,
  ScalingAxis = 1,
  LookUp = 2,
  IntervalAxis = 3,
  ClassificationResult = 4
};

enum class ArrayStorage : uint8_t {
  CnTemplate = 0,
  CgTemplate = 1,
  DgTemplate = 2
};

namespace CaFlag {
constexpr uint32_t DynamicSize = 0x0001;
constexpr uint32_t InputQuantity = 0x0002;
constexpr uint32_t OutputQuantity = 0x0004;
constexpr uint32_t ComparisonQuantity = 0x0008;
constexpr uint32_t Axis = 0x0010;
constexpr uint32_t FixedAxis = 0x0020;
constexpr uint32_t InverseLayout = 0x0040;
constexpr uint32_t LeftOpenInterval = 0x0080;
constexpr uint32_t StandardAxis = 0x0100;
}  // namespace CaFlag

class IChannelArray : public IBlock {
 public:

  virtual void Type(ArrayType type) = 0;
  [[nodiscard]] virtual ArrayType Type() const = 0;

  virtual void Storage(ArrayStorage storage) = 0;
  [[nodiscard]] virtual ArrayStorage Storage() const = 0;

  virtual void Flags(uint32_t flags) = 0;
  [[nodiscard]] virtual uint32_t Flags() const = 0;
};

}  // namespace mdf
