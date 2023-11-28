/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>

#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelConversion {
 private:
  mdf::IChannelConversion* conversion;

 public:
  MdfChannelConversion(mdf::IChannelConversion* conversion)
      : conversion(conversion) {
    if (conversion == nullptr)
      throw std::runtime_error("MdfChannelConversionInit failed");
  }
  MdfChannelConversion(const mdf::IChannelConversion* conversion)
      : MdfChannelConversion(const_cast<mdf::IChannelConversion*>(conversion)) {
  }
  ~MdfChannelConversion() { conversion = nullptr; }
  int64_t GetIndex() const { return MdfChannelConversionGetIndex(conversion); }
  std::string GetName() const {
    size_t size = MdfChannelConversionGetName(conversion, nullptr);
    char* str = new char[size + 1];
    MdfChannelConversionGetName(conversion, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetName(const char* name) {
    MdfChannelConversionSetName(conversion, name);
  }
  std::string GetDescription() const {
    size_t size = MdfChannelConversionGetDescription(conversion, nullptr);
    char* str = new char[size + 1];
    MdfChannelConversionGetDescription(conversion, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDescription(const char* desc) {
    MdfChannelConversionSetDescription(conversion, desc);
  }
  std::string GetUnit() const {
    size_t size = MdfChannelConversionGetUnit(conversion, nullptr);
    char* str = new char[size + 1];
    MdfChannelConversionGetUnit(conversion, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetUnit(const char* unit) {
    MdfChannelConversionSetUnit(conversion, unit);
  }
  ConversionType GetType() const {
    return MdfChannelConversionGetType(conversion);
  }
  void SetType(ConversionType type) {
    MdfChannelConversionSetType(conversion, type);
  }
  bool IsPrecisionUsed() {
    return MdfChannelConversionIsPrecisionUsed(conversion);
  }
  uint8_t GetPrecision() const {
    return MdfChannelConversionGetPrecision(conversion);
  }
  bool IsRangeUsed() { return MdfChannelConversionIsRangeUsed(conversion); }
  double GetRangeMin() const {
    return MdfChannelConversionGetRangeMin(conversion);
  }
  double GetRangeMax() const {
    return MdfChannelConversionGetRangeMax(conversion);
  }
  void SetRange(double min, double max) {
    MdfChannelConversionSetRange(conversion, min, max);
  }
  uint16_t GetFlags() const { return MdfChannelConversionGetFlags(conversion); }
  const MdfChannelConversion GetInverse() const {
    return MdfChannelConversion(MdfChannelConversionGetInverse(conversion));
  }
  MdfChannelConversion CreateInverse() {
    return MdfChannelConversion(MdfChannelConversionCreateInverse(conversion));
  }
};
}  // namespace MdfLibrary