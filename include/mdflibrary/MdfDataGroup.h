/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfChannelGroup.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfDataGroup {
 private:
  mdf::IDataGroup* group;

 public:
  MdfDataGroup(mdf::IDataGroup* group) : group(group) {
    if (group == nullptr) throw std::runtime_error("MdfDataGroupInit failed");
  }
  MdfDataGroup(const mdf::IDataGroup* group)
      : MdfDataGroup(const_cast<mdf::IDataGroup*>(group)) {}
  ~MdfDataGroup() { group = nullptr; }
  mdf::IDataGroup* GetDataGroup() const { return group; }
  int64_t GetIndex() const { return MdfDataGroupGetIndex(group); }
  std::string GetDescription() const {
    size_t size = MdfDataGroupGetDescription(group, nullptr);
    char* str = new char[size + 1];
    MdfDataGroupGetDescription(group, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  uint8_t GetRecordIdSize() const { return MdfDataGroupGetRecordIdSize(group); }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfDataGroupGetMetaData(group));
  }
  std::vector<MdfChannelGroup> GetChannelGroups() const {
    size_t count = MdfDataGroupGetChannelGroups(group, nullptr);
    if (count <= 0) return std::vector<MdfChannelGroup>();
    auto pGroups = new mdf::IChannelGroup*[count];
    MdfDataGroupGetChannelGroups(group, pGroups);
    std::vector<MdfChannelGroup> groups;
    for (size_t i = 0; i < count; i++)
      groups.push_back(MdfChannelGroup(pGroups[i]));
    delete[] pGroups;
    return groups;
  }
  bool IsRead() { return MdfDataGroupIsRead(group); }
  MdfMetaData CreateMetaData() {
    return MdfMetaData(MdfDataGroupCreateMetaData(group));
  }
  MdfChannelGroup CreateChannelGroup() {
    return MdfChannelGroup(MdfDataGroupCreateChannelGroup(group));
  }
  const MdfChannelGroup FindParentChannelGroup(MdfChannel channel) {
    return MdfChannelGroup(
        MdfDataGroupFindParentChannelGroup(group, channel.GetChannel()));
  }
  void ResetSample() { MdfDataGroupResetSample(group); }
};
}  // namespace MdfLibrary