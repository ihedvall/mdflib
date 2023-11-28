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
    size_t size = MdfETagGetName(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetName(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetName(const char* name) { MdfETagSetName(eTag, name); }
  std::string GetDescription() const {
    size_t size = MdfETagGetDescription(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetDescription(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDescription(const char* desc) { MdfETagSetDescription(eTag, desc); }
  std::string GetUnit() const {
    size_t size = MdfETagGetUnit(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetUnit(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetUnit(const char* unit) { MdfETagSetUnit(eTag, unit); }
  std::string GetUnitRef() const {
    size_t size = MdfETagGetUnitRef(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetUnitRef(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetUnitRef(const char* unit) { MdfETagSetUnitRef(eTag, unit); }
  std::string GetType() const {
    size_t size = MdfETagGetType(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetType(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetType(const char* type) { MdfETagSetType(eTag, type); }
  ETagDataType GetDataType() const { return MdfETagGetDataType(eTag); }
  void SetDataType(ETagDataType type) { MdfETagSetDataType(eTag, type); }
  std::string GetLanguage() const {
    size_t size = MdfETagGetLanguage(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetLanguage(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetLanguage(const char* language) { MdfETagSetLanguage(eTag, language); }
  bool GetReadOnly() const { return MdfETagGetReadOnly(eTag); }
  void SetReadOnly(bool read_only) { MdfETagSetReadOnly(eTag, read_only); }
  std::string GetValueAsString() const {
    size_t size = MdfETagGetValueAsString(eTag, nullptr);
    char* str = new char[size + 1];
    MdfETagGetValueAsString(eTag, str);
    std::string s(str, size);
    delete str;
    return s;
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