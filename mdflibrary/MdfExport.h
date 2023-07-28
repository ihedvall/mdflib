/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */

#include <mdf/idatagroup.h>
#include <mdf/iheader.h>
#include <mdf/mdffactory.h>
#include <mdf/mdfreader.h>
#include <mdf/mdfwriter.h>

using namespace mdf;

#if defined(_MSC_VER)
//  Microsoft
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  __declspec(dllexport) ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__GNUC__)
// GCC
#define EXPORTFUNC(ReturnType, FuncName, ...) \
  __attribute__((visibility("default")))      \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#else
#pragma warning Unknown dynamic link import / export semantics.
#endif

extern "C" {
#define EXPORTINITFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfReader, FuncName, __VA_ARGS__)
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfReader, FuncName, MdfReader* reader, __VA_ARGS__)

EXPORTINITFUNC(MdfReader*, Init, char* filename) {
  return new MdfReader(filename);
}
EXPORTINITFUNC(void, UnInit, MdfReader* reader) { delete reader; }
EXPORTFEATUREFUNC(int64_t, GetIndex) { return reader->Index(); }
EXPORTFEATUREFUNC(bool, IsOk) { return reader->IsOk(); }
EXPORTFEATUREFUNC(const MdfFile*, GetFile) { return reader->GetFile(); }
EXPORTFEATUREFUNC(const IHeader*, GetHeader) { return reader->GetHeader(); }
EXPORTFEATUREFUNC(const IDataGroup*, GetDataGroup, size_t index) {
  return reader->GetDataGroup(index);
}
EXPORTFEATUREFUNC(bool, Open) { return reader->Open(); }
EXPORTFEATUREFUNC(void, Close) { reader->Close(); }
EXPORTFEATUREFUNC(bool, ReadHeader) { return reader->ReadHeader(); }
EXPORTFEATUREFUNC(bool, ReadMeasurementInfo) {
  return reader->ReadMeasurementInfo();
}
EXPORTFEATUREFUNC(bool, ReadEverythingButData) {
  return reader->ReadEverythingButData();
}
EXPORTFEATUREFUNC(bool, ReadData, IDataGroup* group) {
  return reader->ReadData(*group);
}
#undef EXPORTINITFUNC
#undef EXPORTFEATUREFUNC

#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfHeader, FuncName, IHeader* header, __VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return header->Index(); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return header->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  header->Description(desc);
}
EXPORTFEATUREFUNC(const char*, GetAuthor) { return header->Author().c_str(); }
EXPORTFEATUREFUNC(void, SetAuthor, const char* author) {
  header->Author(author);
}
EXPORTFEATUREFUNC(const char*, GetDepartment) {
  return header->Department().c_str();
}
EXPORTFEATUREFUNC(void, SetDepartment, const char* department) {
  header->Department(department);
}
EXPORTFEATUREFUNC(const char*, GetProject) { return header->Project().c_str(); }
EXPORTFEATUREFUNC(void, SetProject, const char* project) {
  header->Project(project);
}
EXPORTFEATUREFUNC(const char*, GetSubject) { return header->Subject().c_str(); }
EXPORTFEATUREFUNC(void, SetSubject, const char* subject) {
  header->Subject(subject);
}
EXPORTFEATUREFUNC(const char*, GetMeasurementId) {
  return header->MeasurementId().c_str();
}
EXPORTFEATUREFUNC(void, SetMeasurementId, const char* uuid) {
  header->MeasurementId(uuid);
}
EXPORTFEATUREFUNC(const char*, GetRecorderId) {
  return header->RecorderId().c_str();
}
EXPORTFEATUREFUNC(void, SetRecorderId, const char* uuid) {
  header->RecorderId(uuid);
}
EXPORTFEATUREFUNC(int64_t, GetRecorderIndex) { return header->RecorderIndex(); }
EXPORTFEATUREFUNC(void, SetRecorderIndex, int64_t index) {
  header->RecorderIndex(index);
}
EXPORTFEATUREFUNC(uint64_t, GetStartTime) { return header->StartTime(); }
EXPORTFEATUREFUNC(void, SetStartTime, uint64_t time) {
  header->StartTime(time);
}
EXPORTFEATUREFUNC(bool, IsStartAngleUsed) {
  return header->StartAngle().has_value();
}
EXPORTFEATUREFUNC(double, GetStartAngle) {
  return header->StartAngle().value_or(0.0);
}
EXPORTFEATUREFUNC(void, SetStartAngle, double angle) {
  header->StartAngle(angle);
}
EXPORTFEATUREFUNC(bool, IsStartDistanceUsed) {
  return header->StartDistance().has_value();
}
EXPORTFEATUREFUNC(double, GetStartDistance) {
  return header->StartDistance().value_or(0.0);
}
EXPORTFEATUREFUNC(void, SetStartDistance, double distance) {
  header->StartDistance(distance);
}
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) { return header->MetaData(); }
EXPORTFEATUREFUNC(size_t, GetAttachments, const IAttachment*& pAttachments) {
  pAttachments = header->Attachments()[0];
  return header->Attachments().size();
}
EXPORTFEATUREFUNC(size_t, GetFileHistory, const IFileHistory*& pFileHistorys) {
  pFileHistorys = header->FileHistories()[0];
  return header->FileHistories().size();
}
EXPORTFEATUREFUNC(size_t, GetEvents, const IEvent*& pEvents) {
  pEvents = header->Events()[0];
  return header->Events().size();
}
EXPORTFEATUREFUNC(size_t, GetDataGroups, const IDataGroup*& pDataGroups) {
  pDataGroups = header->DataGroups()[0];
  return header->DataGroups().size();
}
EXPORTFEATUREFUNC(const IAttachment*, CreateAttachment) {
  return header->CreateAttachment();
}
EXPORTFEATUREFUNC(const IFileHistory*, CreateFileHistory) {
  return header->CreateFileHistory();
}
#undef CreateEvent
EXPORTFEATUREFUNC(const IEvent*, CreateEvent) { return header->CreateEvent(); }
EXPORTFEATUREFUNC(const IDataGroup*, CreateDataGroup) {
  return header->CreateDataGroup();
}
#undef EXPORTFEATUREFUNC

#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfDataGroup, FuncName, IDataGroup* group, __VA_ARGS__)

EXPORTFEATUREFUNC(int64_t, GetIndex) { return group->Index(); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return group->Description().c_str();
}
EXPORTFEATUREFUNC(uint8_t, GetRecordIdSize) { return group->RecordIdSize(); }
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) { return group->MetaData(); }
EXPORTFEATUREFUNC(size_t, GetChannelGroups,
                  const IChannelGroup*& pChannelGroups) {
  pChannelGroups = group->ChannelGroups()[0];
  return group->ChannelGroups().size();
}
EXPORTFEATUREFUNC(bool, IsRead) { return group->IsRead(); }
EXPORTFEATUREFUNC(const IMetaData*, CreateMetaData) {
  return group->CreateMetaData();
}
EXPORTFEATUREFUNC(const IChannelGroup*, CreateChannelGroup) {
  return group->CreateChannelGroup();
}
EXPORTFEATUREFUNC(const IChannelGroup*, FindParentChannelGroup,
                  IChannel* channel) {
  return group->FindParentChannelGroup(*channel);
}
EXPORTFEATUREFUNC(void, ResetSample) { group->ResetSample(); }
#undef EXPORTFEATUREFUNC

#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                  \
  EXPORT(ReturnType, MdfChannelGroup, FuncName, IChannelGroup* group, \
         __VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return group->Index(); }
EXPORTFEATUREFUNC(uint64_t, GetRecordId) { return group->RecordId(); }
EXPORTFEATUREFUNC(const char*, GetName) { return group->Name().c_str(); }
EXPORTFEATUREFUNC(void, SetName, const char* name) { group->Name(name); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return group->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  group->Description(desc);
}
EXPORTFEATUREFUNC(uint64_t, GetNofSamples) { return group->NofSamples(); }
EXPORTFEATUREFUNC(void, SetNofSamples, uint64_t samples) {
  group->NofSamples(samples);
}
EXPORTFEATUREFUNC(uint16_t, GetFlags) { return group->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint16_t flags) { group->Flags(flags); }
EXPORTFEATUREFUNC(wchar_t, GetPathSeparator) { return group->PathSeparator(); }
EXPORTFEATUREFUNC(void, SetPathSeparator, wchar_t sep) {
  group->PathSeparator(sep);
}
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) { return group->MetaData(); }
EXPORTFEATUREFUNC(size_t, GetChannels, const IChannel*& pChannels) {
  pChannels = group->Channels()[0];
  return group->Channels().size();
}
EXPORTFEATUREFUNC(const ISourceInformation*, GetSourceInformation) {
  return group->SourceInformation();
}
EXPORTFEATUREFUNC(const IChannel*, GetXChannel, const IChannel* ref_channel) {
  return group->GetXChannel(*ref_channel);
}
EXPORTFEATUREFUNC(const IMetaData*, CreateMetaData) {
  return group->CreateMetaData();
}
EXPORTFEATUREFUNC(const IChannel*, CreateChannel) {
  return group->CreateChannel();
}
EXPORTFEATUREFUNC(const ISourceInformation*, CreateSourceInformation) {
  return group->CreateSourceInformation();
}
#undef EXPORTFEATUREFUNC

#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannel, FuncName, IChannel* channel, __VA_ARGS__)

EXPORTFEATUREFUNC(int64_t, GetIndex) { return channel->Index(); }
EXPORTFEATUREFUNC(const char*, GetName) { return channel->Name().c_str(); }
EXPORTFEATUREFUNC(void, SetName, char* name) { channel->Name(name); }
EXPORTFEATUREFUNC(const char*, GetDisplayName) {
  return channel->DisplayName().c_str();
}
EXPORTFEATUREFUNC(void, SetDisplayName, char* name) {
  channel->DisplayName(name);
}
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return channel->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, char* desc) {
  channel->Description(desc);
}
EXPORTFEATUREFUNC(bool, IsUnitUsed) { return channel->IsUnitValid(); }
EXPORTFEATUREFUNC(const char*, GetUnit) { return channel->Unit().c_str(); }
EXPORTFEATUREFUNC(void, SetUnit, char* unit) { channel->Unit(unit); }
EXPORTFEATUREFUNC(ChannelType, GetType) { return channel->Type(); }
EXPORTFEATUREFUNC(void, SetType, ChannelType type) { channel->Type(type); }
EXPORTFEATUREFUNC(ChannelSyncType, GetSync) { return channel->Sync(); }
EXPORTFEATUREFUNC(void, SetSync, ChannelSyncType type) { channel->Sync(type); }
EXPORTFEATUREFUNC(ChannelDataType, GetDataType) { return channel->DataType(); }
EXPORTFEATUREFUNC(void, SetDataType, ChannelDataType type) {
  channel->DataType(type);
}
EXPORTFEATUREFUNC(uint32_t, GetFlags) { return channel->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint32_t flags) { channel->Flags(flags); }
EXPORTFEATUREFUNC(size_t, GetDataBytes) { return channel->DataBytes(); }
EXPORTFEATUREFUNC(void, SetDataBytes, size_t bytes) {
  channel->DataBytes(bytes);
}
EXPORTFEATUREFUNC(bool, IsPrecisionUsed) { return channel->IsDecimalUsed(); }
EXPORTFEATUREFUNC(uint8_t, GetPrecision) { return channel->Decimals(); }
EXPORTFEATUREFUNC(bool, IsRangeUsed) { return channel->Range().has_value(); }
EXPORTFEATUREFUNC(double, GetRangeMin) { return channel->Range()->first; }
EXPORTFEATUREFUNC(double, GetRangeMax) { return channel->Range()->second; }
EXPORTFEATUREFUNC(bool, IsLimitUsed) { return channel->Limit().has_value(); }
EXPORTFEATUREFUNC(double, GetLimitMin) { return channel->Limit()->first; }
EXPORTFEATUREFUNC(double, GetLimitMax) { return channel->Limit()->second; }
EXPORTFEATUREFUNC(bool, IsExtLimitUsed) {
  return channel->ExtLimit().has_value();
}
EXPORTFEATUREFUNC(double, GetExtLimitMin) { return channel->ExtLimit()->first; }
EXPORTFEATUREFUNC(double, GetExtLimitMax) {
  return channel->ExtLimit()->second;
}
EXPORTFEATUREFUNC(double, GetSamplingRate) { return channel->SamplingRate(); }
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) { return channel->MetaData(); }
EXPORTFEATUREFUNC(const ISourceInformation*, GetSourceInformation) {
  return channel->SourceInformation();
}
EXPORTFEATUREFUNC(const IChannelConversion*, GetChannelConversion) {
  return channel->ChannelConversion();
}
EXPORTFEATUREFUNC(const IMetaData*, CreateMetaData) {
  return channel->CreateMetaData();
}
EXPORTFEATUREFUNC(const ISourceInformation*, CreateSourceInformation) {
  return channel->CreateSourceInformation();
}
EXPORTFEATUREFUNC(const IChannelConversion*, CreateChannelConversion) {
  return channel->CreateChannelConversion();
}
EXPORTFEATUREFUNC(void, SetChannelValueAsSigned, const int64_t value,
                  bool valid = true) {
  channel->SetChannelValue(value, valid);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsUnSigned, const uint64_t value,
                  bool valid = true) {
  channel->SetChannelValue(value, valid);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsFloat, const double value,
                  bool valid = true) {
  channel->SetChannelValue(value, valid);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsString, const char* value,
                  bool valid = true) {
  channel->SetChannelValue(std::string(value), valid);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsArray, const uint8_t* value,
                  size_t size, bool valid = true) {
  channel->SetChannelValue(std::vector<uint8_t>(value, value + size), valid);
}
#undef EXPORTFEATUREFUNC

#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                           \
  EXPORT(ReturnType, MdfChannelConversion, FuncName, IChannelConversion* conv, \
         __VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return conv->Index(); }
EXPORTFEATUREFUNC(const char*, GetName) { return conv->Name().c_str(); }
EXPORTFEATUREFUNC(void, SetName, char* name) { conv->Name(name); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return conv->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, char* desc) { conv->Description(desc); }
EXPORTFEATUREFUNC(const char*, GetUnit) { return conv->Unit().c_str(); }
EXPORTFEATUREFUNC(void, SetUnit, char* unit) { conv->Unit(unit); }
EXPORTFEATUREFUNC(ConversionType, GetType) { return conv->Type(); }
EXPORTFEATUREFUNC(void, SetType, ConversionType type) { conv->Type(type); }
EXPORTFEATUREFUNC(bool, IsPrecisionUsed) { return conv->IsDecimalUsed(); }
EXPORTFEATUREFUNC(uint8_t, GetPrecision) { return conv->Decimals(); }
EXPORTFEATUREFUNC(bool, IsRangeUsed) { return conv->Range().has_value(); }
EXPORTFEATUREFUNC(double, GetRangeMin) { return conv->Range()->first; }
EXPORTFEATUREFUNC(double, GetRangeMax) { return conv->Range()->second; }
EXPORTFEATUREFUNC(uint16_t, GetFlags) { return conv->Flags(); }
EXPORTFEATUREFUNC(const IChannelConversion*, GetInverse) {
  return conv->Inverse();
}
EXPORTFEATUREFUNC(const IChannelConversion*, CreateInverse) {
  return conv->CreateInverse();
}

#undef EXPORTFEATUREFUNC

#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                          \
  EXPORT(ReturnType, MdfChannelObserver, FuncName, IChannelObserver* channel, \
         __VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetNofSamples) { return channel->NofSamples(); }
EXPORTFEATUREFUNC(const char*, GetName) { return channel->Name().c_str(); }
EXPORTFEATUREFUNC(const char*, GetUnit) { return channel->Unit().c_str(); }
EXPORTFEATUREFUNC(const IChannel*, GetChannel) { return &(channel->Channel()); }
EXPORTFEATUREFUNC(bool, IsMaster) { return channel->IsMaster(); }
EXPORTFEATUREFUNC(bool, GetChannelValueAsSigned, uint64_t sample,
                   int64_t& value) {
  return channel->GetChannelValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsUnSigned, uint64_t sample,
                   uint64_t& value) {
  return channel->GetChannelValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsFloat, uint64_t sample,
                   double& value) {
  return channel->GetChannelValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsString, uint64_t sample,
                  char*& value) {
  std::string str;
  bool valid = channel->GetChannelValue(sample, str);
  strcpy(value, str.c_str());
  return valid;
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsArray, uint64_t sample,
                  uint8_t*& value, size_t& size) {
  std::vector<uint8_t> vec;
  bool valid = channel->GetChannelValue(sample, vec);
  size = vec.size();
  if (size > 0) memcpy(value, vec.data(), size);
  return valid;
}
EXPORTFEATUREFUNC(bool, GetEngValueAsSigned, uint64_t sample,
                   int64_t& value) {
  return channel->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsUnSigned, uint64_t sample,
                   uint64_t& value) {
  return channel->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsFloat, uint64_t sample,
                   double& value) {
  return channel->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsString, uint64_t sample, char*& value) {
  std::string str;
  bool valid = channel->GetEngValue(sample, str);
  strcpy(value, str.c_str());
  return valid;
}
EXPORTFEATUREFUNC(bool, GetEngValueAsArray, uint64_t sample, uint8_t*& value,
                  size_t& size) {
  // Ref MdfChannelObserver::GetChannelValueAsArray
  // Note that engineering value cannot be byte arrays so I assume
  // that it was the channel value that was requested.
  std::vector<uint8_t> vec;
  bool valid = channel->GetChannelValue(sample, vec);
  size = vec.size();
  if (size > 0) memcpy(value, vec.data(), size);
  return valid;
}
#undef EXPORTFEATUREFUNC

#pragma region SourceInformation
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfSourceInformation, FuncName, \
         ISourceInformation* source_information, __VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return source_information->Index(); }
EXPORTFEATUREFUNC(const char*, GetName) {
  return source_information->Name().c_str();
}
EXPORTFEATUREFUNC(void, SetName, char* name) { source_information->Name(name); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return source_information->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, char* desc) {
  source_information->Description(desc);
}
EXPORTFEATUREFUNC(const char*, GetPath) {
  return source_information->Path().c_str();
}
EXPORTFEATUREFUNC(void, SetPath, char* path) { source_information->Path(path); }
EXPORTFEATUREFUNC(SourceType, GetType) { return source_information->Type(); }
EXPORTFEATUREFUNC(void, SetType, SourceType type) {
  source_information->Type(type);
}
EXPORTFEATUREFUNC(BusType, GetBus) { return source_information->Bus(); }
EXPORTFEATUREFUNC(void, SetBus, BusType bus) { source_information->Bus(bus); }
EXPORTFEATUREFUNC(uint8_t, GetFlags) { return source_information->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint8_t flags) {
  source_information->Flags(flags);
}
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) {
  return source_information->MetaData();
}
EXPORTFEATUREFUNC(const IMetaData*, CreateMetaData) {
  return source_information->CreateMetaData();
}
#undef EXPORTFEATUREFUNC
#pragma endregion
}