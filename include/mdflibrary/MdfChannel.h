/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfChannelConversion.h"
#include "MdfSourceInformation.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannel {
 private:
  mdf::IChannel* channel;

 public:
  MdfChannel(mdf::IChannel* channel) : channel(channel) {
    if (channel == nullptr) throw std::runtime_error("MdfChannelInit failed");
  }
  MdfChannel(const mdf::IChannel* channel)
      : MdfChannel(const_cast<mdf::IChannel*>(channel)) {}
  ~MdfChannel() { channel = nullptr; }
  mdf::IChannel* GetChannel() const { return channel; }
  int64_t GetIndex() const { return MdfChannelGetIndex(channel); }
  std::string GetName() const {
    size_t size = MdfChannelGetName(channel, nullptr);
    char* str = new char[size + 1];
    MdfChannelGetName(channel, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetName(const char* name) { MdfChannelSetName(channel, name); }
  std::string GetDisplayName() const {
    size_t size = MdfChannelGetDisplayName(channel, nullptr);
    char* str = new char[size + 1];
    MdfChannelGetDisplayName(channel, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDisplayName(const char* name) {
    MdfChannelSetDisplayName(channel, name);
  }
  std::string GetDescription() const {
    size_t size = MdfChannelGetDescription(channel, nullptr);
    char* str = new char[size + 1];
    MdfChannelGetDescription(channel, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDescription(const char* desc) {
    MdfChannelSetDescription(channel, desc);
  }
  bool IsUnitUsed() { return MdfChannelIsUnitUsed(channel); }
  std::string GetUnit() const {
    size_t size = MdfChannelGetUnit(channel, nullptr);
    char* str = new char[size + 1];
    MdfChannelGetUnit(channel, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetUnit(const char* unit) { MdfChannelSetUnit(channel, unit); }
  ChannelType GetType() const { return MdfChannelGetType(channel); }
  void SetType(ChannelType type) { MdfChannelSetType(channel, type); }
  ChannelSyncType GetSync() const { return MdfChannelGetSync(channel); }
  void SetSync(ChannelSyncType type) { MdfChannelSetSync(channel, type); }
  ChannelDataType GetDataType() const { return MdfChannelGetDataType(channel); }
  void SetDataType(ChannelDataType type) {
    MdfChannelSetDataType(channel, type);
  }
  uint32_t GetFlags() const { return MdfChannelGetFlags(channel); }
  void SetFlags(uint32_t flags) { MdfChannelSetFlags(channel, flags); }
  size_t GetDataBytes() const { return MdfChannelGetDataBytes(channel); }
  void SetDataBytes(size_t bytes) { MdfChannelSetDataBytes(channel, bytes); }
  bool IsPrecisionUsed() { return MdfChannelIsPrecisionUsed(channel); }
  uint8_t GetPrecision() const { return MdfChannelGetPrecision(channel); }
  bool IsRangeUsed() { return MdfChannelIsRangeUsed(channel); }
  double GetRangeMin() const { return MdfChannelGetRangeMin(channel); }
  double GetRangeMax() const { return MdfChannelGetRangeMax(channel); }
  void SetRange(double min, double max) {
    MdfChannelSetRange(channel, min, max);
  }
  bool IsLimitUsed() { return MdfChannelIsLimitUsed(channel); }
  double GetLimitMin() const { return MdfChannelGetLimitMin(channel); }
  double GetLimitMax() const { return MdfChannelGetLimitMax(channel); }
  void SetLimit(double min, double max) {
    MdfChannelSetLimit(channel, min, max);
  }
  bool IsExtLimitUsed() { return MdfChannelIsExtLimitUsed(channel); }
  double GetExtLimitMin() const { return MdfChannelGetExtLimitMin(channel); }
  double GetExtLimitMax() const { return MdfChannelGetExtLimitMax(channel); }
  void SetExtLimit(double min, double max) {
    MdfChannelSetExtLimit(channel, min, max);
  }
  double GetSamplingRate() const { return MdfChannelGetSamplingRate(channel); }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfChannelGetMetaData(channel));
  }
  const MdfSourceInformation GetSourceInformation() const {
    return MdfSourceInformation(MdfChannelGetSourceInformation(channel));
  }
  const MdfChannelConversion GetChannelConversion() const {
    return MdfChannelConversion(MdfChannelGetChannelConversion(channel));
  }
  MdfMetaData CreateMetaData() {
    return MdfMetaData(MdfChannelCreateMetaData(channel));
  }
  MdfSourceInformation CreateSourceInformation() {
    return MdfSourceInformation(MdfChannelCreateSourceInformation(channel));
  }
  MdfChannelConversion CreateChannelConversion() {
    return MdfChannelConversion(MdfChannelCreateChannelConversion(channel));
  }
  void SetChannelValue(const int64_t value, bool valid = true) {
    MdfChannelSetChannelValueAsSigned(channel, value, valid);
  }
  void SetChannelValue(const uint64_t value, bool valid = true) {
    MdfChannelSetChannelValueAsUnSigned(channel, value, valid);
  }
  void SetChannelValue(const double value, bool valid = true) {
    MdfChannelSetChannelValueAsFloat(channel, value, valid);
  }
  void SetChannelValue(const char* value, bool valid = true) {
    MdfChannelSetChannelValueAsString(channel, value, valid);
  }
  void SetChannelValue(const uint8_t* value, size_t size, bool valid = true) {
    MdfChannelSetChannelValueAsArray(channel, value, size, valid);
  }
};
}  // namespace MdfLibrary
