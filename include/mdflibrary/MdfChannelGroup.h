#pragma once
#include "MdfChannel.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelGroup {
 private:
  const mdf::IChannelGroup* group;

 public:
  MdfChannelGroup(const mdf::IChannelGroup* group) : group(group) {}
  ~MdfChannelGroup() { group = nullptr; }
  const mdf::IChannelGroup* GetChannelGroup() { return group; }
  int64_t GetIndex() { return MdfChannelGroupGetIndex(group); }
  uint64_t GetRecordId() { return MdfChannelGroupGetRecordId(group); }
  const char* GetName() { return MdfChannelGroupGetName(group); }
  void SetName(const char* name) { MdfChannelGroupSetName(group, name); }
  const char* GetDescription() { return MdfChannelGroupGetDescription(group); }
  void SetDescription(const char* desc) {
    MdfChannelGroupSetDescription(group, desc);
  }
  uint64_t GetNofSamples() { return MdfChannelGroupGetNofSamples(group); }
  void SetNofSamples(uint64_t samples) {
    MdfChannelGroupSetNofSamples(group, samples);
  }
  uint16_t GetFlags() { return MdfChannelGroupGetFlags(group); }
  void SetFlags(uint16_t flags) { MdfChannelGroupSetFlags(group, flags); }
  wchar_t GetPathSeparator() { return MdfChannelGroupGetPathSeparator(group); }
  void SetPathSeparator(wchar_t sep) {
    MdfChannelGroupSetPathSeparator(group, sep);
  }
  const MdfMetaData GetMetaData() {
    return MdfMetaData(MdfChannelGroupGetMetaData(group));
  }
  std::vector<MdfChannel> GetChannels() {
    size_t count = MdfChannelGroupGetChannels(group, nullptr);
    if (count <= 0) return std::vector<MdfChannel>();
    auto pChannels = new mdf::IChannel*[count];
    MdfChannelGroupGetChannels(group, pChannels);
    std::vector<MdfChannel> channels;
    for (size_t i = 0; i < count; i++) channels.push_back (MdfChannel(pChannels[i]));
    return channels;
  }
  const MdfSourceInformation GetSourceInformation() {
    return MdfSourceInformation(MdfChannelGroupGetSourceInformation(group));
  }
  const MdfChannel GetXChannel(MdfChannel ref_channel) {
    return MdfChannel(
        MdfChannelGroupGetXChannel(group, ref_channel.GetChannel()));
  }
  MdfMetaData CreateMetaData() {
    return MdfMetaData(MdfChannelGroupCreateMetaData(group));
  }
  MdfChannel CreateChannel() {
    return MdfChannel(MdfChannelGroupCreateChannel(group));
  }
  MdfSourceInformation CreateSourceInformation() {
    return MdfSourceInformation(MdfChannelGroupCreateSourceInformation(group));
  }
};
}  // namespace MdfLibrary