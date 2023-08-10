#pragma once
#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfETag {
 private:
  const mdf::ETag* eTag;

 public:
  MdfETag(const mdf::ETag* eTag) : eTag(eTag) {}
  ~MdfETag() { eTag = nullptr; }
  const mdf::ETag* GetETag() { return eTag; }
  const char* GetName() { return MdfETagGetName(eTag); }
  void SetName(const char* name) { SetName(name); }
  const char* GetDescription() { return MdfETagGetDescription(eTag); }
  void SetDescription(const char* desc) { SetDescription(desc); }
  const char* GetUnit() { return MdfETagGetUnit(eTag); }
  void SetUnit(const char* unit) { SetUnit(unit); }
  const char* GetUnitRef() { return MdfETagGetUnitRef(eTag); }
  void SetUnitRef(const char* unit) { SetUnitRef(unit); }
  const char* GetType() { return MdfETagGetType(eTag); }
  void SetType(const char* type) { SetType(type); }
  ETagDataType GetDataType() { return MdfETagGetDataType(eTag); }
  void SetDataType(ETagDataType type) { SetDataType(type); }
  const char* GetLanguage() { return MdfETagGetLanguage(eTag); }
  void SetLanguage(const char* language) { SetLanguage(language); }
  bool GetReadOnly() { return MdfETagGetReadOnly(eTag); }
  void SetReadOnly(bool read_only) { SetReadOnly(read_only); }
  const char* GetValueAsString() { return MdfETagGetValueAsString(eTag); }
  void SetValueAsString(const char* value) { SetValueAsString(value); }
  double GetValueAsFloat() { return MdfETagGetValueAsFloat(eTag); }
  void SetValueAsFloat(double value) { SetValueAsFloat(value); }
  bool GetValueAsBoolean() { return MdfETagGetValueAsBoolean(eTag); }
  void SetValueAsBoolean(bool value) { SetValueAsBoolean(value); }
  int64_t GetValueAsSigned() { return MdfETagGetValueAsSigned(eTag); }
  void SetValueAsSigned(int64_t value) { SetValueAsSigned(value); }
  uint64_t GetValueAsUnsigned() { return MdfETagGetValueAsUnsigned(eTag); }
  void SetValueAsUnsigned(uint64_t value) { SetValueAsUnsigned(value); }
};
}  // namespace MdfLibrary