/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */

#include <mdf/etag.h>
#include <mdf/iattachment.h>
#include <mdf/idatagroup.h>
#include <mdf/ievent.h>
#include <mdf/ifilehistory.h>
#include <mdf/mdffactory.h>
#include <mdf/mdfreader.h>
#include <mdf/mdfwriter.h>

using namespace mdf;

#if defined(_WIN32)
// WINDOWS
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  __declspec(dllexport) ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__linux__)
// LINUX
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  __attribute__((visibility("default")))             \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#else
#pragma warning Unknown dynamic link import / export semantics.
#endif

extern "C" {
#pragma region MdfReader
#define EXPORTINITFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfReader, FuncName, __VA_ARGS__)
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfReader, FuncName, MdfReader* reader, ##__VA_ARGS__)
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
#pragma endregion

#pragma region MdfWriter
#define EXPORTINITFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfWriter, FuncName, ##__VA_ARGS__)
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfWriter, FuncName, MdfWriter* writer, ##__VA_ARGS__)
EXPORTINITFUNC(MdfWriter*, Init, MdfWriterType type, char* filename) {
  auto* writer = MdfFactory::CreateMdfWriterEx(type);
  if (!writer) return nullptr;
  writer->Init(filename);
  return writer;
}
EXPORTINITFUNC(void, UnInit, MdfWriter* writer) { delete writer; }
EXPORTFEATUREFUNC(MdfFile*, GetFile) { return writer->GetFile(); }
EXPORTFEATUREFUNC(IHeader*, GetHeader) { return writer->Header(); }
EXPORTFEATUREFUNC(bool, GetCompressData) { return writer->CompressData(); }
EXPORTFEATUREFUNC(void, SetCompressData, bool compress) {
  writer->CompressData(compress);
}
EXPORTFEATUREFUNC(IDataGroup*, CreateDataGroup) {
  return writer->CreateDataGroup();
}
EXPORTFEATUREFUNC(bool, InitMeasurement) { return writer->InitMeasurement(); }
EXPORTFEATUREFUNC(void, SaveSample, IChannelGroup* group, uint64_t time) {
  writer->SaveSample(*group, time);
}
EXPORTFEATUREFUNC(void, StartMeasurement, uint64_t start_time) {
  writer->StartMeasurement(start_time);
}
EXPORTFEATUREFUNC(void, StopMeasurement, uint64_t stop_time) {
  writer->StopMeasurement(stop_time);
}
EXPORTFEATUREFUNC(bool, FinalizeMeasurement) {
  return writer->FinalizeMeasurement();
}
#undef EXPORTINITFUNC
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFile
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfFile, FuncName, MdfFile* file, ##__VA_ARGS__)
EXPORTFEATUREFUNC(size_t, GetAttachments, const IAttachment**& pAttachment) {
  AttachmentList AttachmentList;
  file->Attachments(AttachmentList);
  pAttachment = &AttachmentList[0];
  return AttachmentList.size();
}
EXPORTFEATUREFUNC(size_t, GetDataGroups, const IDataGroup**& pDataGroup) {
  DataGroupList DataGroupList;
  file->DataGroups(DataGroupList);
  pDataGroup = &DataGroupList[0];
  return DataGroupList.size();
}
EXPORTFEATUREFUNC(const char*, GetName) { return file->Name().c_str(); }
EXPORTFEATUREFUNC(void, SetName, char* name) { file->Name(name); }
EXPORTFEATUREFUNC(const char*, GetFileName) { return file->FileName().c_str(); }
EXPORTFEATUREFUNC(void, SetFileName, char* filename) {
  file->FileName(filename);
}
EXPORTFEATUREFUNC(const char*, GetVersion) { return file->Version().c_str(); }
EXPORTFEATUREFUNC(int, GetMainVersion) { return file->MainVersion(); }
EXPORTFEATUREFUNC(int, GetMinorVersion) { return file->MinorVersion(); }
EXPORTFEATUREFUNC(void, SetMinorVersion, int minor) {
  file->MinorVersion(minor);
}
EXPORTFEATUREFUNC(const char*, GetProgramId) {
  return file->ProgramId().c_str();
}
EXPORTFEATUREFUNC(void, SetProgramId, char* program_id) {
  file->ProgramId(program_id);
}
EXPORTFEATUREFUNC(bool, GetFinalized, uint16_t& standard_flags,
                  uint16_t& custom_flags) {
  return file->IsFinalized(standard_flags, custom_flags);
}
EXPORTFEATUREFUNC(const IHeader*, GetHeader) { return file->Header(); }
EXPORTFEATUREFUNC(bool, GetIsMdf4) { return file->IsMdf4(); }
EXPORTFEATUREFUNC(IAttachment*, CreateAttachment) {
  return file->CreateAttachment();
}
EXPORTFEATUREFUNC(IDataGroup*, CreateDataGroup) {
  return file->CreateDataGroup();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfHeader
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfHeader, FuncName, IHeader* header, ##__VA_ARGS__)
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
EXPORTFEATUREFUNC(const IMetaData*, GetMetaDatas) { return header->MetaData(); }
EXPORTFEATUREFUNC(size_t, GetAttachments, IAttachment**& pAttachments) {
  auto attachments = header->Attachments();
  pAttachments = &attachments[0];
  return attachments.size();
}
EXPORTFEATUREFUNC(size_t, GetFileHistorys, IFileHistory**& pFileHistorys) {
  auto file_historys = header->FileHistories();
  pFileHistorys = &file_historys[0];
  return file_historys.size();
}
EXPORTFEATUREFUNC(size_t, GetEvents, IEvent**& pEvents) {
  auto events = header->Events();
  pEvents = &events[0];
  return events.size();
}
EXPORTFEATUREFUNC(size_t, GetDataGroups, IDataGroup**& pDataGroups) {
  auto data_groups = header->DataGroups();
  pDataGroups = &data_groups[0];
  return data_groups.size();
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
#pragma endregion

#pragma region MdfDataGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfDataGroup, FuncName, IDataGroup* group, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return group->Index(); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return group->Description().c_str();
}
EXPORTFEATUREFUNC(uint8_t, GetRecordIdSize) { return group->RecordIdSize(); }
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) { return group->MetaData(); }
EXPORTFEATUREFUNC(size_t, GetChannelGroups, IChannelGroup**& pChannelGroups) {
  auto channel_groups = group->ChannelGroups();
  pChannelGroups = &channel_groups[0];
  return channel_groups.size();
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
#pragma endregion

#pragma region MdfChannelGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                  \
  EXPORT(ReturnType, MdfChannelGroup, FuncName, IChannelGroup* group, \
         ##__VA_ARGS__)
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
EXPORTFEATUREFUNC(size_t, GetChannels, IChannel**& pChannels) {
  auto channels = group->Channels();
  pChannels = &channels[0];
  return channels.size();
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
#pragma endregion

#pragma region MdfChannel
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannel, FuncName, IChannel* channel, ##__VA_ARGS__)

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
#pragma endregion

#pragma region MdfChannelConversion
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                           \
  EXPORT(ReturnType, MdfChannelConversion, FuncName, IChannelConversion* conv, \
         ##__VA_ARGS__)
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
#pragma endregion

#pragma region MdfChannelObserver
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                          \
  EXPORT(ReturnType, MdfChannelObserver, FuncName, IChannelObserver* channel, \
         ##__VA_ARGS__)
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
EXPORTFEATUREFUNC(bool, GetEngValueAsSigned, uint64_t sample, int64_t& value) {
  return channel->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsUnSigned, uint64_t sample,
                  uint64_t& value) {
  return channel->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsFloat, uint64_t sample, double& value) {
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
  // Ref `MdfChannelObserver::GetChannelValueAsArray`
  // Note that engineering value cannot be byte arrays so I assume
  // that it was the channel value that was requested.
  std::vector<uint8_t> vec;
  bool valid = channel->GetChannelValue(sample, vec);
  size = vec.size();
  if (size > 0) memcpy(value, vec.data(), size);
  return valid;
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfSourceInformation
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfSourceInformation, FuncName, \
         ISourceInformation* source_information, ##__VA_ARGS__)
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

#pragma region MdfAttachment
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                   \
  EXPORT(ReturnType, MdfAttachment, FuncName, IAttachment* attachment, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return attachment->Index(); }
EXPORTFEATUREFUNC(uint16_t, GetCreatorIndex) {
  return attachment->CreatorIndex();
}
EXPORTFEATUREFUNC(void, SetCreatorIndex, uint16_t index) {
  attachment->CreatorIndex(index);
}
EXPORTFEATUREFUNC(bool, GetEmbedded) { return attachment->IsEmbedded(); }
EXPORTFEATUREFUNC(void, SetEmbedded, bool embedded) {
  attachment->IsEmbedded(embedded);
}
EXPORTFEATUREFUNC(bool, GetCompressed) { return attachment->IsCompressed(); }
EXPORTFEATUREFUNC(void, SetCompressed, bool compressed) {
  attachment->IsCompressed(compressed);
}
EXPORTFEATUREFUNC(const char*, GetMd5) {
  return attachment->Md5().value_or(std::string()).c_str();
}
EXPORTFEATUREFUNC(const char*, GetFileName) {
  return attachment->FileName().c_str();
}
EXPORTFEATUREFUNC(void, SetFileName, char* name) { attachment->FileName(name); }
EXPORTFEATUREFUNC(const char*, GetFileType) {
  return attachment->FileType().c_str();
}
EXPORTFEATUREFUNC(void, SetFileType, char* type) { attachment->FileType(type); }
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFileHistory
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                       \
  EXPORT(ReturnType, MdfFileHistory, FuncName, IFileHistory* file_history, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return file_history->Index(); }
EXPORTFEATUREFUNC(uint64_t, GetTime) { return file_history->Time(); }
EXPORTFEATUREFUNC(void, SetTime, uint64_t time) { file_history->Time(time); }
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) {
  return file_history->MetaData();
}
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return file_history->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, char* desc) {
  file_history->Description(desc);
}
EXPORTFEATUREFUNC(const char*, GetToolName) {
  return file_history->ToolName().c_str();
}
EXPORTFEATUREFUNC(void, SetToolName, char* name) {
  file_history->ToolName(name);
}
EXPORTFEATUREFUNC(const char*, GetToolVendor) {
  return file_history->ToolVendor().c_str();
}
EXPORTFEATUREFUNC(void, SetToolVendor, char* vendor) {
  file_history->ToolVendor(vendor);
}
EXPORTFEATUREFUNC(const char*, GetToolVersion) {
  return file_history->ToolVersion().c_str();
}
EXPORTFEATUREFUNC(void, SetToolVersion, char* version) {
  file_history->ToolVersion(version);
}
EXPORTFEATUREFUNC(const char*, GetUserName) {
  return file_history->UserName().c_str();
}
EXPORTFEATUREFUNC(void, SetUserName, char* user) {
  file_history->UserName(user);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfEvent
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfEvent, FuncName, IEvent* event, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return event->Index(); }
EXPORTFEATUREFUNC(const char*, GetName) { return event->Name().c_str(); }
EXPORTFEATUREFUNC(void, SetName, char* name) { event->Name(name); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return event->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, char* desc) {
  event->Description(desc);
}
EXPORTFEATUREFUNC(const char*, GetGroupName) {
  return event->GroupName().c_str();
}
EXPORTFEATUREFUNC(void, SetGroupName, char* group) { event->GroupName(group); }
EXPORTFEATUREFUNC(EventType, GetType) { return event->Type(); }
EXPORTFEATUREFUNC(void, SetType, EventType type) { event->Type(type); }
EXPORTFEATUREFUNC(SyncType, GetSync) { return event->Sync(); }
EXPORTFEATUREFUNC(void, SetSync, SyncType type) { event->Sync(type); }
EXPORTFEATUREFUNC(RangeType, GetRange) { return event->Range(); }
EXPORTFEATUREFUNC(void, SetRange, RangeType type) { event->Range(type); }
EXPORTFEATUREFUNC(EventCause, GetCause) { return event->Cause(); }
EXPORTFEATUREFUNC(void, SetCause, EventCause cause) { event->Cause(cause); }
EXPORTFEATUREFUNC(int64_t, GetCreatorIndex) { return event->CreatorIndex(); }
EXPORTFEATUREFUNC(void, SetCreatorIndex, int64_t index) {
  event->CreatorIndex(index);
}
EXPORTFEATUREFUNC(int64_t, GetSyncValue) { return event->SyncValue(); }
EXPORTFEATUREFUNC(void, SetSyncValue, int64_t value) {
  event->SyncValue(value);
}
EXPORTFEATUREFUNC(double, GetSyncFactor) { return event->SyncFactor(); }
EXPORTFEATUREFUNC(void, SetSyncFactor, double factor) {
  event->SyncFactor(factor);
}
EXPORTFEATUREFUNC(const IEvent*, GetParentEvent) {
  return event->ParentEvent();
}
EXPORTFEATUREFUNC(void, SetParentEvent, IEvent* parent) {
  event->ParentEvent(parent);
}
EXPORTFEATUREFUNC(const IEvent*, GetRangeEvent) { return event->RangeEvent(); }
EXPORTFEATUREFUNC(void, SetRangeEvent, IEvent* range) {
  event->RangeEvent(range);
}
EXPORTFEATUREFUNC(size_t, GetAttachments, const IAttachment**& pAttachment) {
  auto attachment = event->Attachments();
  pAttachment = &attachment[0];
  return attachment.size();
}
EXPORTFEATUREFUNC(double, GetPreTrig) { return event->PreTrig(); }
EXPORTFEATUREFUNC(void, SetPreTrig, double time) { event->PreTrig(time); }
EXPORTFEATUREFUNC(double, GetPostTrig) { return event->PostTrig(); }
EXPORTFEATUREFUNC(void, SetPostTrig, double time) { event->PostTrig(time); }
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData) { return event->MetaData(); }
EXPORTFEATUREFUNC(void, AddAttachment, IAttachment* attachment) {
  event->AddAttachment(attachment);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfETag
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfETag, FuncName, ETag* etag, ##__VA_ARGS__)
EXPORTFEATUREFUNC(const char*, GetName) { return etag->Name().c_str(); }
EXPORTFEATUREFUNC(void, SetName, char* name) { etag->Name(name); }
EXPORTFEATUREFUNC(const char*, GetDescription) {
  return etag->Description().c_str();
}
EXPORTFEATUREFUNC(void, SetDescription, char* desc) { etag->Description(desc); }
EXPORTFEATUREFUNC(const char*, GetUnit) { return etag->Unit().c_str(); }
EXPORTFEATUREFUNC(void, SetUnit, char* unit) { etag->Unit(unit); }
EXPORTFEATUREFUNC(const char*, GetUnitRef) { return etag->UnitRef().c_str(); }
EXPORTFEATUREFUNC(void, SetUnitRef, char* unit) { etag->UnitRef(unit); }
EXPORTFEATUREFUNC(const char*, GetType) { return etag->Type().c_str(); }
EXPORTFEATUREFUNC(void, SetType, char* type) { etag->Type(type); }
EXPORTFEATUREFUNC(ETagDataType, GetDataType) { return etag->DataType(); }
EXPORTFEATUREFUNC(void, SetDataType, ETagDataType type) {
  etag->DataType(type);
}
EXPORTFEATUREFUNC(const char*, GetLanguage) { return etag->Language().c_str(); }
EXPORTFEATUREFUNC(void, SetLanguage, char* language) {
  etag->Language(language);
}
EXPORTFEATUREFUNC(bool, GetReadOnly) { return etag->ReadOnly(); }
EXPORTFEATUREFUNC(void, SetReadOnly, bool read_only) {
  etag->ReadOnly(read_only);
}
EXPORTFEATUREFUNC(const char*, GetValueAsString) {
  return etag->Value<std::string>().c_str();
}
EXPORTFEATUREFUNC(void, SetValueAsString, char* value) {
  etag->Value(std::string(value));
}
EXPORTFEATUREFUNC(double, GetValueAsFloat) { return etag->Value<double>(); }
EXPORTFEATUREFUNC(void, SetValueAsFloat, double value) { etag->Value(value); }
EXPORTFEATUREFUNC(bool, GetValueAsBoolean) { return etag->Value<bool>(); }
EXPORTFEATUREFUNC(void, SetValueAsBoolean, bool value) { etag->Value(value); }
EXPORTFEATUREFUNC(int64_t, GetValueAsSigned) { return etag->Value<int64_t>(); }
EXPORTFEATUREFUNC(void, SetValueAsSigned, int64_t value) { etag->Value(value); }
EXPORTFEATUREFUNC(uint64_t, GetValueAsUnsigned) {
  return etag->Value<uint64_t>();
}
EXPORTFEATUREFUNC(void, SetValueAsUnsigned, uint64_t value) {
  etag->Value(value);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfMetaData
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfMetaData, FuncName, IMetaData* metadata, ##__VA_ARGS__)
EXPORTFEATUREFUNC(const char*, GetPropertyAsString, char* index) {
  return metadata->StringProperty(index).c_str();
}
EXPORTFEATUREFUNC(void, SetPropertyAsString, char* index, char* prop) {
  metadata->StringProperty(index, std::string(prop));
}
EXPORTFEATUREFUNC(double, GetPropertyAsFloat, char* index) {
  return metadata->FloatProperty(index);
}
EXPORTFEATUREFUNC(void, SetPropertyAsFloat, char* index, double prop) {
  metadata->FloatProperty(index, prop);
}
EXPORTFEATUREFUNC(size_t, GetProperties, const ETag*& pProperty) {
  auto properties = metadata->Properties();
  pProperty = &properties[0];
  return properties.size();
}
EXPORTFEATUREFUNC(size_t, GetCommonProperties, const ETag*& pProperty) {
  auto properties = metadata->CommonProperties();
  pProperty = &properties[0];
  return properties.size();
}
EXPORTFEATUREFUNC(void, SetCommonProperties, const ETag* pProperty,
                  size_t count) {
  std::vector<ETag> properties(pProperty, pProperty + count);
  metadata->CommonProperties(properties);
}
EXPORTFEATUREFUNC(const char*, GetXmlSnippet) {
  return metadata->XmlSnippet().c_str();
}
EXPORTFEATUREFUNC(void, SetXmlSnippet, char* xml) {
  metadata->XmlSnippet(std::string(xml));
}
EXPORTFEATUREFUNC(void, AddCommonProperty, ETag* tag) {
  metadata->CommonProperty(*tag);
}
#undef EXPORTFEATUREFUNC
#pragma endregion
}