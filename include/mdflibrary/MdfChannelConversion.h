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
    std::string str;
    size_t size = MdfChannelConversionGetName(conversion, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfChannelConversionGetName(conversion, str.data());
    return str;
  }
  void SetName(const char* name) {
    MdfChannelConversionSetName(conversion, name);
  }
  std::string GetDescription() const {
    std::string str;
    size_t size = MdfChannelConversionGetDescription(conversion, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfChannelConversionGetDescription(conversion, str.data());
    return str;
  }
  void SetDescription(const char* desc) {
    MdfChannelConversionSetDescription(conversion, desc);
  }
  std::string GetUnit() const {
    std::string str;
    size_t size = MdfChannelConversionGetUnit(conversion, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfChannelConversionGetUnit(conversion, str.data());
    return str;
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