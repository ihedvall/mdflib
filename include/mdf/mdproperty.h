/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <type_traits>

#include "mdf/mdstandardattribute.h"

namespace mdf {

enum class MdDataType : uint16_t {
  MdString = 0,
  MdDecimal,
  MdInteger,
  MdFloat,
  MdBoolean,
  MdDate,
  MdTime,
  MdDateTime,
  MdHex, ///< Same as MdInteger but written as hex string
};

class MdProperty;
class IXmlNode;

using MdPropertyList = std::map<std::string, MdProperty>;

class MdProperty : public MdStandardAttribute {
 public:
  MdProperty() = default;
  MdProperty(std::string property, std::string value);

  void Name(std::string name);
  [[nodiscard]] const std::string& Name() const;

  void Description(std::string description);
  [[nodiscard]] const std::string& Description() const;

  void Unit(std::string unit);
  [[nodiscard]] const std::string& Unit() const;

  void UnitReference(std::string unit_reference);
  [[nodiscard]] const std::string& UnitReference() const;

  void DataType(MdDataType data_type);
  [[nodiscard]] MdDataType DataType() const;

  void ReadOnly(bool read_only);
  [[nodiscard]] bool ReadOnly() const;

  template <typename T>
  void Value(T value);

  template <typename T>
  [[nodiscard]] T Value() const;

  void ToXml(IXmlNode& prop_node) const override;
  void FromXml(const IXmlNode& prop_node) override;

 private:
  std::string name_;
  std::string desc_;
  std::string unit_;
  std::string unit_ref_;
  MdDataType data_type_ = MdDataType::MdString;
  bool read_only_ = false;


  std::string value_;

  [[nodiscard]] std::string_view DataTypeToString() const;
  void StringToDataType(const std::string& text);
};

template <typename T>
void MdProperty::Value(T value) {
  if constexpr (std::is_floating_point_v<T>) {
    if (data_type_ == MdDataType::MdFloat) {
      // More digits until the text reads back the same value.
      // Needs DataType() first.
      for (int digits = std::numeric_limits<T>::digits10;
           digits <= std::numeric_limits<T>::max_digits10; ++digits) {
        std::ostringstream temp;
        temp.imbue(std::locale::classic());
        temp << std::setprecision(digits) << value;
        value_ = temp.str();
        std::istringstream check(value_);
        check.imbue(std::locale::classic());
        T read_back = {};
        if (check >> read_back && read_back == value) {
          break;
        }
      }
      return;
    }
  }
  value_ = std::to_string(value);
}

template <>
void MdProperty::Value(std::string value);

template <>
void MdProperty::Value(bool value);

template <>
void MdProperty::Value(uint64_t value);

template <typename T>
T MdProperty::Value() const {
  T output = {};
  if (value_.empty()) {
    return output;
  }

  std::istringstream temp(value_);
  if (data_type_ == MdDataType::MdFloat) {
    temp.imbue(std::locale::classic());
  }
  temp >> output;
  if (data_type_ == MdDataType::MdFloat && temp.peek() == ',') {
    std::istringstream legacy(value_);  // Comma from the global locale
    legacy >> output;
  }
  return output;
}

template <>
std::string MdProperty::Value() const;

template <>
uint64_t MdProperty::Value() const;

template <>
bool MdProperty::Value() const;
}  // namespace mdf

