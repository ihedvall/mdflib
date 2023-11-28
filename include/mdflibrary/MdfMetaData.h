/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>

#include "MdfETag.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfMetaData {
 private:
  mdf::IMetaData* metaData;

 public:
  MdfMetaData(mdf::IMetaData* metaData) : metaData(metaData) {
    if (metaData == nullptr) throw std::runtime_error("MdfMetaDataInit failed");
  }
  MdfMetaData(const mdf::IMetaData* metaData)
      : MdfMetaData(const_cast<mdf::IMetaData*>(metaData)) {}
  ~MdfMetaData() { metaData = nullptr; }
  std::string GetPropertyAsString(const char* index) {
    size_t size = MdfMetaDataGetPropertyAsString(metaData, index, nullptr);
    char* str = new char[size + 1];
    MdfMetaDataGetPropertyAsString(metaData, index, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetPropertyAsString(const char* index, const char* prop) {
    MdfMetaDataSetPropertyAsString(metaData, index, prop);
  }
  double GetPropertyAsFloat(const char* index) {
    return MdfMetaDataGetPropertyAsFloat(metaData, index);
  }
  void SetPropertyAsFloat(const char* index, double prop) {
    MdfMetaDataSetPropertyAsFloat(metaData, index, prop);
  }
  std::vector<MdfETag> GetProperties() const {
    size_t count = MdfMetaDataGetProperties(metaData, nullptr);
    if (count <= 0) return std::vector<MdfETag>();
    auto pTags = new mdf::ETag*[count];
    MdfMetaDataGetProperties(metaData, pTags);
    std::vector<MdfETag> tags;
    for (size_t i = 0; i < count; i++) tags.push_back(MdfETag(pTags[i]));
    delete[] pTags;
    return tags;
  }
  std::vector<MdfETag> GetCommonProperties() const {
    size_t count = MdfMetaDataGetCommonProperties(metaData, nullptr);
    if (count <= 0) return std::vector<MdfETag>();
    auto pTags = new mdf::ETag*[count];
    MdfMetaDataGetCommonProperties(metaData, pTags);
    std::vector<MdfETag> tags;
    for (size_t i = 0; i < count; i++) tags.push_back(MdfETag(pTags[i]));
    delete[] pTags;
    return tags;
  }
  void SetCommonProperties(std::vector<MdfETag> pProperty, size_t count) {
    auto pTags = new const mdf::ETag*[count];
    for (size_t i = 0; i < count; i++) pTags[i] = pProperty[i].GetETag();
    MdfMetaDataSetCommonProperties(metaData, pTags, count);
    delete[] pTags;
  }
  std::string GetXmlSnippet() const {
    size_t size = MdfMetaDataGetXmlSnippet(metaData, nullptr);
    char* str = new char[size + 1];
    MdfMetaDataGetXmlSnippet(metaData, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetXmlSnippet(const char* xml) {
    MdfMetaDataSetXmlSnippet(metaData, xml);
  }
  void AddCommonProperty(MdfETag tag) {
    MdfMetaDataAddCommonProperty(metaData, tag.GetETag());
  }
};
}  // namespace MdfLibrary