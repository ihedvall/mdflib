/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <stdexcept>

#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfETag {
 private:
  mdf::ETag* eTag;

 public:
  MdfETag(mdf::ETag* eTag) : eTag(eTag) {
    if (eTag == nullptr) throw std::runtime_error("MdfETagInit failed");
  }
  MdfETag(const mdf::ETag* eTag) : MdfETag(const_cast<mdf::ETag*>(eTag)) {}
  MdfETag() : MdfETag(MdfETagInit()) {}
  ~MdfETag() {
    if (eTag == nullptr) return;
    MdfETagUnInit(eTag);
    eTag = nullptr;
  }
  MdfETag(const MdfETag&) = delete;
  MdfETag(MdfETag&& eTag) {
    this->eTag = eTag.eTag;
    eTag.eTag = nullptr;
  }
  mdf::ETag* GetETag() const { return eTag; }
  std::string GetName() const {
    std::string str;
    str.reserve(MdfETagGetName(eTag, nullptr) + 1);
    str.resize(MdfETagGetName(eTag, str.data()));;
    return str;
  }
  void SetName(const char* name) { MdfETagSetName(eTag, name); }
  std::string GetDescription() const {
    std::string str;
    str.reserve(MdfETagGetDescription(eTag, nullptr) + 1);
    str.resize(MdfETagGetDescription(eTag, str.data()));;
    return str;
  }
  void SetDescription(const char* desc) { MdfETagSetDescription(eTag, desc); }
  std::string GetUnit() const {
    std::string str;
    str.reserve(MdfETagGetUnit(eTag, nullptr) + 1);
    str.resize(MdfETagGetUnit(eTag, str.data()));;
    return str;
  }
  void SetUnit(const char* unit) { MdfETagSetUnit(eTag, unit); }
  std::string GetUnitRef() const {
    std::string str;
    str.reserve(MdfETagGetUnitRef(eTag, nullptr) + 1);
    str.resize(MdfETagGetUnitRef(eTag, str.data()));;
    return str;
  }
  void SetUnitRef(const char* unit) { MdfETagSetUnitRef(eTag, unit); }
  std::string GetType() const {
    std::string str;
    str.reserve(MdfETagGetType(eTag, nullptr) + 1);
    str.resize(MdfETagGetType(eTag, str.data()));;
    return str;
  }
  void SetType(const char* type) { MdfETagSetType(eTag, type); }
  ETagDataType GetDataType() const { return MdfETagGetDataType(eTag); }
  void SetDataType(ETagDataType type) { MdfETagSetDataType(eTag, type); }
  std::string GetLanguage() const {
    std::string str;
    str.reserve(MdfETagGetLanguage(eTag, nullptr) + 1);
    str.resize(MdfETagGetLanguage(eTag, str.data()));;
    return str;
  }
  void SetLanguage(const char* language) { MdfETagSetLanguage(eTag, language); }
  bool GetReadOnly() const { return MdfETagGetReadOnly(eTag); }
  void SetReadOnly(bool read_only) { MdfETagSetReadOnly(eTag, read_only); }
  std::string GetValueAsString() const {
    std::string str;
    str.reserve(MdfETagGetValueAsString(eTag, nullptr) + 1);
    str.resize(MdfETagGetValueAsString(eTag, str.data()));;
    return str;
  }
  void SetValueAsString(const char* value) {
    MdfETagSetValueAsString(eTag, value);
  }
  double GetValueAsFloat() const { return MdfETagGetValueAsFloat(eTag); }
  void SetValueAsFloat(double value) { MdfETagSetValueAsFloat(eTag, value); }
  bool GetValueAsBoolean() const { return MdfETagGetValueAsBoolean(eTag); }
  void SetValueAsBoolean(bool value) { MdfETagSetValueAsBoolean(eTag, value); }
  int64_t GetValueAsSigned() const { return MdfETagGetValueAsSigned(eTag); }
  void SetValueAsSigned(int64_t value) { MdfETagSetValueAsSigned(eTag, value); }
  uint64_t GetValueAsUnsigned() const {
    return MdfETagGetValueAsUnsigned(eTag);
  }
  void SetValueAsUnsigned(uint64_t value) {
    MdfETagSetValueAsUnsigned(eTag, value);
  }
};
}  // namespace MdfLibrary