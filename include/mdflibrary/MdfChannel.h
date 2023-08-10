#pragma once
#include "MdfChannelConversion.h"
#include "MdfSourceInformation.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannel {
 private:
  const mdf::IChannel* channel;

 public:
  MdfChannel(const mdf::IChannel* channel) { this->channel = channel; }
  ~MdfChannel() { this->channel = nullptr; }
  const mdf::IChannel* GetChannel() { return channel; }
  int64_t GetIndex() { return MdfChannelGetIndex(channel); }
  const char* GetName() { return MdfChannelGetName(channel); }
  void SetName(const char* name) { MdfChannelSetName(channel, name); }
  const char* GetDisplayName() { return MdfChannelGetDisplayName(channel); }
  void SetDisplayName(const char* name) {
    MdfChannelSetDisplayName(channel, name);
  }
  const char* GetDescription() { return MdfChannelGetDescription(channel); }
  void SetDescription(const char* desc) {
    MdfChannelSetDescription(channel, desc);
  }
  bool IsUnitUsed() { return MdfChannelIsUnitUsed(channel); }
  const char* GetUnit() { return MdfChannelGetUnit(channel); }
  void SetUnit(const char* unit) { MdfChannelSetUnit(channel, unit); }
  ChannelType GetType() { return MdfChannelGetType(channel); }
  void SetType(ChannelType type) { MdfChannelSetType(channel, type); }
  ChannelSyncType GetSync() { return MdfChannelGetSync(channel); }
  void SetSync(ChannelSyncType type) { MdfChannelSetSync(channel, type); }
  ChannelDataType GetDataType() { return MdfChannelGetDataType(channel); }
  void SetDataType(ChannelDataType type) {
    MdfChannelSetDataType(channel, type);
  }
  uint32_t GetFlags() { return MdfChannelGetFlags(channel); }
  void SetFlags(uint32_t flags) { MdfChannelSetFlags(channel, flags); }
  size_t GetDataBytes() { return MdfChannelGetDataBytes(channel); }
  void SetDataBytes(size_t bytes) { MdfChannelSetDataBytes(channel, bytes); }
  bool IsPrecisionUsed() { return MdfChannelIsPrecisionUsed(channel); }
  uint8_t GetPrecision() { return MdfChannelGetPrecision(channel); }
  bool IsRangeUsed() { return MdfChannelIsRangeUsed(channel); }
  double GetRangeMin() { return MdfChannelGetRangeMin(channel); }
  double GetRangeMax() { return MdfChannelGetRangeMax(channel); }
  void SetRange(double min, double max) {
    MdfChannelSetRange(channel, min, max);
  }
  bool IsLimitUsed() { return MdfChannelIsLimitUsed(channel); }
  double GetLimitMin() { return MdfChannelGetLimitMin(channel); }
  double GetLimitMax() { return MdfChannelGetLimitMax(channel); }
  void SetLimit(double min, double max) {
    MdfChannelSetLimit(channel, min, max);
  }
  bool IsExtLimitUsed() { return MdfChannelIsExtLimitUsed(channel); }
  double GetExtLimitMin() { return MdfChannelGetExtLimitMin(channel); }
  double GetExtLimitMax() { return MdfChannelGetExtLimitMax(channel); }
  void SetExtLimit(double min, double max) {
    MdfChannelSetExtLimit(channel, min, max);
  }
  double GetSamplingRate() { return MdfChannelGetSamplingRate(channel); }
  const MdfMetaData GetMetaData() {
    return MdfMetaData(MdfChannelGetMetaData(channel));
  }
  const MdfSourceInformation GetSourceInformation() {
    return MdfSourceInformation(MdfChannelGetSourceInformation(channel));
  }
  const MdfChannelConversion GetChannelConversion() {
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
  void SetChannelValueAsSigned(const int64_t value, bool valid = true) {
    MdfChannelSetChannelValueAsSigned(channel, value, valid);
  }
  void SetChannelValueAsUnSigned(const uint64_t value, bool valid = true) {
    MdfChannelSetChannelValueAsUnSigned(channel, value, valid);
  }
  void SetChannelValueAsFloat(const double value, bool valid = true) {
    MdfChannelSetChannelValueAsFloat(channel, value, valid);
  }
  void SetChannelValueAsString(const char* value, bool valid = true) {
    MdfChannelSetChannelValueAsString(channel, value, valid);
  }
  void SetChannelValueAsArray(const uint8_t* value, size_t size,
                              bool valid = true) {
    MdfChannelSetChannelValueAsArray(channel, value, size, valid);
  }
};
}  // namespace MdfLibrary
