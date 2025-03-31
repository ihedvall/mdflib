/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <sstream>

#include "mdf/mdstandardattribute.h"
#include "mdf/mdproperty.h"

namespace mdf {

enum class MdByteOrder {
  LittleEndian,
  BigEndian
};

class MdNumber : public MdStandardAttribute {
 public:
  MdNumber() = default;
  explicit MdNumber(double number, uint64_t history_index = 0,
                    std::string language = {});

  operator double() const; // NOLINT(*-explicit-constructor)

  [[nodiscard]] bool IsActive() const override;

  template <typename T>
  void Number(T value);

  template <typename T>
  [[nodiscard]] T Number() const;

  void DataType(MdDataType type);
  [[nodiscard]] MdDataType DataType() const;

  void Triggered(bool triggered);
  [[nodiscard]] bool Triggered() const;

  void ByteCount(uint64_t byte_count);
  [[nodiscard]] uint64_t ByteCount() const;

  void BitMas(uint64_t bit_mask);
  [[nodiscard]] uint64_t BitMask() const;

  void ByteOrder(MdByteOrder byte_order);
  [[nodiscard]] MdByteOrder ByteOrder() const;

  void Min(double min);
  [[nodiscard]] double Min() const;

  void Max(double max);
  [[nodiscard]] double Max() const;

  void Average(double average);
  [[nodiscard]] double Average() const;

  void Unit(std::string unit);
  [[nodiscard]] const std::string& Unit() const;

  void UnitRef(std::string unit_ref);
  [[nodiscard]] const std::string& UnitRef() const;

 private:
  std::string number_;
  bool triggered_ = false;
  MdDataType data_type_ = MdDataType::MdFloat;
  uint64_t byte_count_ = 0;
  uint64_t bit_mask_ = 0x00;
  MdByteOrder byte_order_ = MdByteOrder::LittleEndian;
  double min_ = 0.0;
  double max_ = 0.0;
  double average_ = 0.0;
  std::string unit_;
  std::string unit_ref_;




};

template <typename T>
void MdNumber::Number(T value) {
  try {
    number_ = std__to_string(value);
  } catch (const std::exception&) {
    number_.clear();
  }
}

template <typename T>
T MdNumber::Number() const {
  T value = {};
  std::istringstream temp(number_);
  temp >> value;
  return value;
}
}  // namespace mdf

