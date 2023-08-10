#pragma once
#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelConversion {
 private:
  const mdf::IChannelConversion* conversion;

 public:
  MdfChannelConversion(const mdf::IChannelConversion* conversion) {
    this->conversion = conversion;
  }
  ~MdfChannelConversion() { this->conversion = nullptr; }
  int64_t GetIndex() { return MdfChannelConversionGetIndex(conversion); }
  const char* GetName() { return MdfChannelConversionGetName(conversion); }
  void SetName(const char* name) {
    MdfChannelConversionSetName(conversion, name);
  }
  const char* GetDescription() {
    return MdfChannelConversionGetDescription(conversion);
  }
  void SetDescription(const char* desc) {
    MdfChannelConversionSetDescription(conversion, desc);
  }
  const char* GetUnit() { return MdfChannelConversionGetUnit(conversion); }
  void SetUnit(const char* unit) {
    MdfChannelConversionSetUnit(conversion, unit);
  }
  ConversionType GetType() { return MdfChannelConversionGetType(conversion); }
  void SetType(ConversionType type) {
    MdfChannelConversionSetType(conversion, type);
  }
  bool IsPrecisionUsed() {
    return MdfChannelConversionIsPrecisionUsed(conversion);
  }
  uint8_t GetPrecision() {
    return MdfChannelConversionGetPrecision(conversion);
  }
  bool IsRangeUsed() { return MdfChannelConversionIsRangeUsed(conversion); }
  double GetRangeMin() { return MdfChannelConversionGetRangeMin(conversion); }
  double GetRangeMax() { return MdfChannelConversionGetRangeMax(conversion); }
  void SetRange(double min, double max) {
    MdfChannelConversionSetRange(conversion, min, max);
  }
  uint16_t GetFlags() { return MdfChannelConversionGetFlags(conversion); }
  const MdfChannelConversion GetInverse() {
    return MdfChannelConversion(MdfChannelConversionGetInverse(conversion));
  }
  MdfChannelConversion CreateInverse() {
    return MdfChannelConversion(MdfChannelConversionCreateInverse(conversion));
  }
};
}  // namespace MdfLibrary