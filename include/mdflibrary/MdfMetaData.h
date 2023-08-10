#pragma once
#include <vector>

#include "MdfETag.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfMetaData {
 private:
  const mdf::IMetaData* metaData;

 public:
  MdfMetaData(const mdf::IMetaData* metaData) : metaData(metaData) {}
  ~MdfMetaData() { metaData = nullptr; }
  const char* GetPropertyAsString(const char* index) {
    return MdfMetaDataGetPropertyAsString(metaData, index);
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
  std::vector<MdfETag> GetProperties() {
    size_t count = MdfMetaDataGetProperties(metaData, nullptr);
    if (count <= 0) return std::vector<MdfETag>();
    auto pTags = new const mdf::ETag*[count];
    MdfMetaDataGetProperties(metaData, pTags);
    std::vector<MdfETag> tags;
    for (size_t i = 0; i < count; i++) tags.push_back(MdfETag(pTags[i]));
    return tags;
  }
  std::vector<MdfETag> GetCommonProperties() {
    size_t count = MdfMetaDataGetCommonProperties(metaData, nullptr);
    if (count <= 0) return std::vector<MdfETag>();
    auto pTags = new const mdf::ETag*[count];
    MdfMetaDataGetCommonProperties(metaData, pTags);
    std::vector<MdfETag> tags;
    for (size_t i = 0; i < count; i++) tags.push_back(MdfETag(pTags[i]));
    return tags;
  }
  void SetCommonProperties(std::vector<MdfETag*> pProperty, size_t count) {
    auto pTags = new const mdf::ETag*[count];
    for (size_t i = 0; i < count; i++) pTags[i] = pProperty[i]->GetETag();
    MdfMetaDataSetCommonProperties(metaData, pTags, count);
  }
  const char* GetXmlSnippet() { return MdfMetaDataGetXmlSnippet(metaData); }
  void SetXmlSnippet(const char* xml) {
    MdfMetaDataSetXmlSnippet(metaData, xml);
  }
  void AddCommonProperty(MdfETag tag) {
    MdfMetaDataAddCommonProperty(metaData, tag.GetETag());
  }
};
}  // namespace MdfLibrary