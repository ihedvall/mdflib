#pragma once
#include "MdfChannelGroup.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfDataGroup {
 private:
  const mdf::IDataGroup* group;

 public:
  MdfDataGroup(const mdf::IDataGroup* group) : group(group) {}
  ~MdfDataGroup() { group = nullptr; }
  const mdf::IDataGroup* GetDataGroup() { return group; }
  int64_t GetIndex() { return MdfDataGroupGetIndex(group); }
  const char* GetDescription() { return MdfDataGroupGetDescription(group); }
  uint8_t GetRecordIdSize() { return MdfDataGroupGetRecordIdSize(group); }
  const MdfMetaData GetMetaData() {
    return MdfMetaData(MdfDataGroupGetMetaData(group));
  }
  std::vector<MdfChannelGroup> GetChannelGroups() {
    size_t count = MdfDataGroupGetChannelGroups(group, nullptr);
    if (count <= 0) return std::vector<MdfChannelGroup>();
    auto pGroups = new mdf::IChannelGroup*[count];
    MdfDataGroupGetChannelGroups(group, pGroups);
    std::vector<MdfChannelGroup> groups;
    for (size_t i = 0; i < count; i++) groups.push_back (MdfChannelGroup(pGroups[i]));
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