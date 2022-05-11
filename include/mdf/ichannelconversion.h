/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include "util/stringutil.h"

namespace mdf {

enum class ConversionType : uint8_t {
  NoConversion = 0,
  Linear = 1,
  Rational = 2,
  Algebraic = 3,
  ValueToValueInterpolation = 4,
  ValueToValue = 5,
  ValueRangeToValue = 6,
  ValueToText = 7,
  ValueRangeToText = 8,
  TextToValue = 9,
  TextToTranslation = 10,

    // MDF 3 types
  Polynomial = 30,
  Exponential = 31,
  Logarithmic = 32,
  DateConversion = 33,
  TimeConversion = 34
};

class IChannelConversion {
 protected:
  uint16_t nof_values_ = 0;
  std::vector<double> value_list_;
  uint8_t channel_data_type_ = 0; ///< The channels data type. Needed by some conversions


  [[nodiscard]] bool IsChannelInteger() const;
  [[nodiscard]] bool IsChannelFloat() const;

  virtual bool ConvertLinear(double channel_value, double& eng_value) const;
  virtual bool ConvertRational(double channel_value, double& eng_value) const;
  virtual bool ConvertAlgebraic(double channel_value, double& eng_value) const;
  virtual bool ConvertValueToValueInterpolate(double channel_value, double& eng_value) const;
  virtual bool ConvertValueToValue(double channel_value, double& eng_value) const;
  virtual bool ConvertValueRangeToValue(double channel_value, double& eng_value) const;

  virtual bool ConvertValueToText(double channel_value, std::string& eng_value) const;
  virtual bool ConvertValueRangeToText(double channel_value, std::string& eng_value) const;
  virtual bool ConvertTextToValue(const std::string& channel_value, double& eng_value) const;
  virtual bool ConvertTextToTranslation(const std::string& channel_value, std::string& eng_value) const;
  virtual bool ConvertPolynomial(double channel_value, double& eng_value) const;
  virtual bool ConvertLogarithmic(double channel_value, double& eng_value) const;
  virtual bool ConvertExponential(double channel_value, double& eng_value) const;
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  [[nodiscard]] virtual std::string Name() const = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  virtual void Unit(const std::string& unit) = 0;
  [[nodiscard]] virtual std::string Unit() const = 0;
  [[nodiscard]] virtual bool IsUnitValid() const = 0;

  virtual void Type(ConversionType type) = 0;
  [[nodiscard]] virtual ConversionType Type() const = 0;

  [[nodiscard]] virtual bool IsDecimalUsed() const = 0;
  [[nodiscard]] virtual uint8_t Decimals() const = 0;

  void Parameter(size_t index, double parameter);

  void ChannelDataType(uint8_t channel_data_type);

  template<typename T, typename V>
  bool Convert(const T& channel_value, V& eng_value) const {
    bool valid = false;
    double value = 0.0;
    switch (Type()) {
      case ConversionType::Linear: {
        valid = ConvertLinear(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::Rational: {
        valid = ConvertRational(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::Algebraic: {
        valid = ConvertAlgebraic(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::ValueToValueInterpolation: {
        valid = ConvertValueToValueInterpolate(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::ValueToValue: {
        valid = ConvertValueToValue(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::ValueRangeToValue: {
        valid = ConvertValueRangeToValue(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::ValueToText: {
        std::string text;
        valid = ConvertValueToText(static_cast<double>(channel_value),text);
        std::istringstream s(text);
        s >> eng_value;
        break;
      }

      case ConversionType::ValueRangeToText: {
        std::string text;
        valid = ConvertValueRangeToText(static_cast<double>(channel_value),text);
        std::istringstream s(text);
        s >> eng_value;
        break;
      }


      case ConversionType::Polynomial: {
        valid = ConvertPolynomial(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::Exponential: {
        valid = ConvertExponential(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }

      case ConversionType::Logarithmic: {
        valid = ConvertLogarithmic(static_cast<double>(channel_value),value);
        eng_value = static_cast<V>(value);
        break;
      }
      case ConversionType::NoConversion:
      default:{
        eng_value = static_cast<V>(channel_value);
        valid = true;
        break;
      }
    }
    return valid;
  }

  template<typename T, typename V = std::string>
  bool Convert(const T& channel_value, std::string& eng_value) const {
    bool valid = false;
    double value = 0.0;
    switch (Type()) {
      case ConversionType::Linear: {
        valid = ConvertLinear(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
        break;
      }

      case ConversionType::Rational: {
        valid = ConvertRational(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
        break;
      }

      case ConversionType::Algebraic: {
        valid = ConvertAlgebraic(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
        break;
      }

      case ConversionType::ValueToValueInterpolation: {
        valid = ConvertValueToValueInterpolate(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
        break;
      }

      case ConversionType::ValueToValue: {
        valid = ConvertValueToValue(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
        break;
      }

      case ConversionType::ValueRangeToValue: {
        valid = ConvertValueRangeToValue(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);
        break;
      }

      case ConversionType::ValueToText: {
        valid = ConvertValueToText(static_cast<double>(channel_value),eng_value);
        break;
      }

      case ConversionType::ValueRangeToText: {
        valid = ConvertValueRangeToText(static_cast<double>(channel_value),eng_value);
        break;
      }

      case ConversionType::Polynomial: {
        valid = ConvertPolynomial(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);;
        break;
      }

      case ConversionType::Exponential: {
        valid = ConvertExponential(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);;
        break;
      }

      case ConversionType::Logarithmic: {
        valid = ConvertLogarithmic(static_cast<double>(channel_value),value);
        eng_value = util::string::FormatDouble(value, IsDecimalUsed() ? Decimals() : 6);;
        break;
      }

      case ConversionType::NoConversion:
      default:{
        eng_value = util::string::FormatDouble(static_cast<double>(channel_value), IsDecimalUsed() ? Decimals() : 6);
        valid = true;
        break;
      }
    }
    return valid;
  }

  template<typename T = std::string, typename V = double>
  bool Convert(const std::string& channel_value, double& eng_value) const {
    if (Type() == ConversionType::TextToValue) {
      ConvertTextToValue(channel_value, eng_value );
    } else if (Type() == ConversionType::NoConversion) {
      eng_value = std::stod(channel_value);
    } else {
      return false;
    }
    return true;
  }

  template<typename T = std::string, typename V = std::string>
  bool Convert(const std::string& channel_value, std::string& eng_value) const {
    if (Type() == ConversionType::TextToTranslation) {
      ConvertTextToTranslation(channel_value, eng_value );
    } else if (Type() == ConversionType::NoConversion) {
      eng_value = std::stod(channel_value);
    } else {
      return false;
    }
    return true;
  }
};

}
