/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfChannel.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelGroup {
 private:
  mdf::IChannelGroup* group;

 public:
  MdfChannelGroup(mdf::IChannelGroup* group) : group(group) {
    if (group == nullptr)
      throw std::runtime_error("MdfChannelGroupInit failed");
  }
  MdfChannelGroup(const mdf::IChannelGroup* group)
      : MdfChannelGroup(const_cast<mdf::IChannelGroup*>(group)) {}
  ~MdfChannelGroup() { group = nullptr; }
  mdf::IChannelGroup* GetChannelGroup() const { return group; }
  int64_t GetIndex() const { return MdfChannelGroupGetIndex(group); }
  uint64_t GetRecordId() const { return MdfChannelGroupGetRecordId(group); }
  std::string GetName() const {
    size_t size = MdfChannelGroupGetName(group, nullptr);
    char* str = new char[size + 1];
    MdfChannelGroupGetName(group, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetName(const char* name) { MdfChannelGroupSetName(group, name); }
  std::string GetDescription() const {
    size_t size = MdfChannelGroupGetDescription(group, nullptr);
    char* str = new char[size + 1];
    MdfChannelGroupGetDescription(group, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDescription(const char* desc) {
    MdfChannelGroupSetDescription(group, desc);
  }
  uint64_t GetNofSamples() const { return MdfChannelGroupGetNofSamples(group); }
  void SetNofSamples(uint64_t samples) {
    MdfChannelGroupSetNofSamples(group, samples);
  }
  uint16_t GetFlags() const { return MdfChannelGroupGetFlags(group); }
  void SetFlags(uint16_t flags) { MdfChannelGroupSetFlags(group, flags); }
  wchar_t GetPathSeparator() const {
    return MdfChannelGroupGetPathSeparator(group);
  }
  void SetPathSeparator(wchar_t sep) {
    MdfChannelGroupSetPathSeparator(group, sep);
  }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfChannelGroupGetMetaData(group));
  }
  std::vector<MdfChannel> GetChannels() const {
    size_t count = MdfChannelGroupGetChannels(group, nullptr);
    if (count <= 0) return std::vector<MdfChannel>();
    auto pChannels = new mdf::IChannel*[count];
    MdfChannelGroupGetChannels(group, pChannels);
    std::vector<MdfChannel> channels;
    for (size_t i = 0; i < count; i++)
      channels.push_back(MdfChannel(pChannels[i]));
    delete[] pChannels;
    return channels;
  }
  const MdfSourceInformation GetSourceInformation() const {
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