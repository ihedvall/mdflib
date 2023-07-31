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
EXPORTINITFUNC(MdfReader*, Init, const char* filename);
EXPORTINITFUNC(void, UnInit, MdfReader* reader);
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(bool, IsOk);
EXPORTFEATUREFUNC(const MdfFile*, GetFile);
EXPORTFEATUREFUNC(const IHeader*, GetHeader);
EXPORTFEATUREFUNC(const IDataGroup*, GetDataGroup, size_t index);
EXPORTFEATUREFUNC(bool, Open);
EXPORTFEATUREFUNC(void, Close);
EXPORTFEATUREFUNC(bool, ReadHeader);
EXPORTFEATUREFUNC(bool, ReadMeasurementInfo);
EXPORTFEATUREFUNC(bool, ReadEverythingButData);
EXPORTFEATUREFUNC(bool, ReadData, IDataGroup* group);
#undef EXPORTINITFUNC
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfWriter
#define EXPORTINITFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfWriter, FuncName, ##__VA_ARGS__)
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfWriter, FuncName, MdfWriter* writer, ##__VA_ARGS__)
EXPORTINITFUNC(MdfWriter*, Init, MdfWriterType type, const char* filename);
EXPORTINITFUNC(void, UnInit, MdfWriter* writer);
EXPORTFEATUREFUNC(MdfFile*, GetFile);
EXPORTFEATUREFUNC(IHeader*, GetHeader);
EXPORTFEATUREFUNC(bool, GetCompressData);
EXPORTFEATUREFUNC(void, SetCompressData, bool compress);
EXPORTFEATUREFUNC(IDataGroup*, CreateDataGroup);
EXPORTFEATUREFUNC(bool, InitMeasurement);
EXPORTFEATUREFUNC(void, SaveSample, IChannelGroup* group, uint64_t time);
EXPORTFEATUREFUNC(void, StartMeasurement, uint64_t start_time);
EXPORTFEATUREFUNC(void, StopMeasurement, uint64_t stop_time);
EXPORTFEATUREFUNC(bool, FinalizeMeasurement);
#undef EXPORTINITFUNC
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFile
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfFile, FuncName, MdfFile* file, ##__VA_ARGS__)
EXPORTFEATUREFUNC(size_t, GetAttachments, const IAttachment**& pAttachment);
EXPORTFEATUREFUNC(size_t, GetDataGroups, const IDataGroup**& pDataGroup);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetFileName);
EXPORTFEATUREFUNC(void, SetFileName, const char* filename);
EXPORTFEATUREFUNC(const char*, GetVersion);
EXPORTFEATUREFUNC(int, GetMainVersion);
EXPORTFEATUREFUNC(int, GetMinorVersion);
EXPORTFEATUREFUNC(void, SetMinorVersion, int minor);
EXPORTFEATUREFUNC(const char*, GetProgramId);
EXPORTFEATUREFUNC(void, SetProgramId, const char* program_id);
EXPORTFEATUREFUNC(bool, GetFinalized, uint16_t& standard_flags,
                  uint16_t& custom_flags);
EXPORTFEATUREFUNC(const IHeader*, GetHeader);
EXPORTFEATUREFUNC(bool, GetIsMdf4);
EXPORTFEATUREFUNC(IAttachment*, CreateAttachment);
EXPORTFEATUREFUNC(IDataGroup*, CreateDataGroup);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfHeader
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfHeader, FuncName, IHeader* header, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(const char*, GetAuthor);
EXPORTFEATUREFUNC(void, SetAuthor, const char* author);
EXPORTFEATUREFUNC(const char*, GetDepartment);
EXPORTFEATUREFUNC(void, SetDepartment, const char* department);
EXPORTFEATUREFUNC(const char*, GetProject);
EXPORTFEATUREFUNC(void, SetProject, const char* project);
EXPORTFEATUREFUNC(const char*, GetSubject);
EXPORTFEATUREFUNC(void, SetSubject, const char* subject);
EXPORTFEATUREFUNC(const char*, GetMeasurementId);
EXPORTFEATUREFUNC(void, SetMeasurementId, const char* uuid);
EXPORTFEATUREFUNC(const char*, GetRecorderId);
EXPORTFEATUREFUNC(void, SetRecorderId, const char* uuid);
EXPORTFEATUREFUNC(int64_t, GetRecorderIndex);
EXPORTFEATUREFUNC(void, SetRecorderIndex, int64_t index);
EXPORTFEATUREFUNC(uint64_t, GetStartTime);
EXPORTFEATUREFUNC(void, SetStartTime, uint64_t time);
EXPORTFEATUREFUNC(bool, IsStartAngleUsed);
EXPORTFEATUREFUNC(double, GetStartAngle);
EXPORTFEATUREFUNC(void, SetStartAngle, double angle);
EXPORTFEATUREFUNC(bool, IsStartDistanceUsed);
EXPORTFEATUREFUNC(double, GetStartDistance);
EXPORTFEATUREFUNC(void, SetStartDistance, double distance);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaDatas);
EXPORTFEATUREFUNC(size_t, GetAttachments, IAttachment**& pAttachments);
EXPORTFEATUREFUNC(size_t, GetFileHistorys, IFileHistory**& pFileHistorys);
EXPORTFEATUREFUNC(size_t, GetEvents, IEvent**& pEvents);
EXPORTFEATUREFUNC(size_t, GetDataGroups, IDataGroup**& pDataGroups);
EXPORTFEATUREFUNC(IAttachment*, CreateAttachment);
EXPORTFEATUREFUNC(IFileHistory*, CreateFileHistory);
#undef CreateEvent
EXPORTFEATUREFUNC(IEvent*, CreateEvent);
EXPORTFEATUREFUNC(IDataGroup*, CreateDataGroup);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfDataGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfDataGroup, FuncName, IDataGroup* group, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(uint8_t, GetRecordIdSize);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(size_t, GetChannelGroups, IChannelGroup**& pChannelGroups);
EXPORTFEATUREFUNC(bool, IsRead);
EXPORTFEATUREFUNC(IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(IChannelGroup*, CreateChannelGroup);
EXPORTFEATUREFUNC(const IChannelGroup*, FindParentChannelGroup,
                  IChannel* channel);
EXPORTFEATUREFUNC(void, ResetSample);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                  \
  EXPORT(ReturnType, MdfChannelGroup, FuncName, IChannelGroup* group, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(uint64_t, GetRecordId);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(uint64_t, GetNofSamples);
EXPORTFEATUREFUNC(void, SetNofSamples, uint64_t samples);
EXPORTFEATUREFUNC(uint16_t, GetFlags);
EXPORTFEATUREFUNC(void, SetFlags, uint16_t flags);
EXPORTFEATUREFUNC(wchar_t, GetPathSeparator);
EXPORTFEATUREFUNC(void, SetPathSeparator, wchar_t sep);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(size_t, GetChannels, IChannel**& pChannels);
EXPORTFEATUREFUNC(const ISourceInformation*, GetSourceInformation);
EXPORTFEATUREFUNC(const IChannel*, GetXChannel, const IChannel* ref_channel);
EXPORTFEATUREFUNC(IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(IChannel*, CreateChannel);
EXPORTFEATUREFUNC(ISourceInformation*, CreateSourceInformation);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannel
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannel, FuncName, IChannel* channel, ##__VA_ARGS__)

EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDisplayName);
EXPORTFEATUREFUNC(void, SetDisplayName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(bool, IsUnitUsed);
EXPORTFEATUREFUNC(const char*, GetUnit);
EXPORTFEATUREFUNC(void, SetUnit, const char* unit);
EXPORTFEATUREFUNC(ChannelType, GetType);
EXPORTFEATUREFUNC(void, SetType, ChannelType type);
EXPORTFEATUREFUNC(ChannelSyncType, GetSync);
EXPORTFEATUREFUNC(void, SetSync, ChannelSyncType type);
EXPORTFEATUREFUNC(ChannelDataType, GetDataType);
EXPORTFEATUREFUNC(void, SetDataType, ChannelDataType type);
EXPORTFEATUREFUNC(uint32_t, GetFlags);
EXPORTFEATUREFUNC(void, SetFlags, uint32_t flags);
EXPORTFEATUREFUNC(size_t, GetDataBytes);
EXPORTFEATUREFUNC(void, SetDataBytes, size_t bytes);
EXPORTFEATUREFUNC(bool, IsPrecisionUsed);
EXPORTFEATUREFUNC(uint8_t, GetPrecision);
EXPORTFEATUREFUNC(bool, IsRangeUsed);
EXPORTFEATUREFUNC(double, GetRangeMin);
EXPORTFEATUREFUNC(double, GetRangeMax);
EXPORTFEATUREFUNC(void, SetRange, double min, double max);
EXPORTFEATUREFUNC(bool, IsLimitUsed);
EXPORTFEATUREFUNC(double, GetLimitMin);
EXPORTFEATUREFUNC(double, GetLimitMax);
EXPORTFEATUREFUNC(void, SetLimit, double min, double max);
EXPORTFEATUREFUNC(bool, IsExtLimitUsed);
EXPORTFEATUREFUNC(double, GetExtLimitMin);
EXPORTFEATUREFUNC(double, GetExtLimitMax);
EXPORTFEATUREFUNC(void, SetExtLimit, double min, double max);
EXPORTFEATUREFUNC(double, GetSamplingRate);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(const ISourceInformation*, GetSourceInformation);
EXPORTFEATUREFUNC(const IChannelConversion*, GetChannelConversion);
EXPORTFEATUREFUNC(IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(ISourceInformation*, CreateSourceInformation);
EXPORTFEATUREFUNC(IChannelConversion*, CreateChannelConversion);
EXPORTFEATUREFUNC(void, SetChannelValueAsSigned, const int64_t value,
                  bool valid = true);
EXPORTFEATUREFUNC(void, SetChannelValueAsUnSigned, const uint64_t value,
                  bool valid = true);
EXPORTFEATUREFUNC(void, SetChannelValueAsFloat, const double value,
                  bool valid = true);
EXPORTFEATUREFUNC(void, SetChannelValueAsString, const char* value,
                  bool valid = true);
EXPORTFEATUREFUNC(void, SetChannelValueAsArray, const uint8_t* value,
                  size_t size, bool valid = true);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelConversion
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                           \
  EXPORT(ReturnType, MdfChannelConversion, FuncName, IChannelConversion* conv, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(const char*, GetUnit);
EXPORTFEATUREFUNC(void, SetUnit, const char* unit);
EXPORTFEATUREFUNC(ConversionType, GetType);
EXPORTFEATUREFUNC(void, SetType, ConversionType type);
EXPORTFEATUREFUNC(bool, IsPrecisionUsed);
EXPORTFEATUREFUNC(uint8_t, GetPrecision);
EXPORTFEATUREFUNC(bool, IsRangeUsed);
EXPORTFEATUREFUNC(double, GetRangeMin);
EXPORTFEATUREFUNC(double, GetRangeMax);
EXPORTFEATUREFUNC(void, SetRange, double min, double max);
EXPORTFEATUREFUNC(uint16_t, GetFlags);
EXPORTFEATUREFUNC(const IChannelConversion*, GetInverse);
EXPORTFEATUREFUNC(IChannelConversion*, CreateInverse);

#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelObserver
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                          \
  EXPORT(ReturnType, MdfChannelObserver, FuncName, IChannelObserver* channel, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetNofSamples);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(const char*, GetUnit);
EXPORTFEATUREFUNC(const IChannel*, GetChannel);
EXPORTFEATUREFUNC(bool, IsMaster);
EXPORTFEATUREFUNC(bool, GetChannelValueAsSigned, uint64_t sample,
                  int64_t& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsUnSigned, uint64_t sample,
                  uint64_t& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsFloat, uint64_t sample, double& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsString, uint64_t sample, char*& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsArray, uint64_t sample,
                  uint8_t*& value, size_t& size);
EXPORTFEATUREFUNC(bool, GetEngValueAsSigned, uint64_t sample, int64_t& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsUnSigned, uint64_t sample,
                  uint64_t& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsFloat, uint64_t sample, double& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsString, uint64_t sample, char*& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsArray, uint64_t sample, uint8_t*& value,
                  size_t& size);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfSourceInformation
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfSourceInformation, FuncName, \
         ISourceInformation* source_information, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(const char*, GetPath);
EXPORTFEATUREFUNC(void, SetPath, const char* path);
EXPORTFEATUREFUNC(SourceType, GetType);
EXPORTFEATUREFUNC(void, SetType, SourceType type);
EXPORTFEATUREFUNC(BusType, GetBus);
EXPORTFEATUREFUNC(void, SetBus, BusType bus);
EXPORTFEATUREFUNC(uint8_t, GetFlags);
EXPORTFEATUREFUNC(void, SetFlags, uint8_t flags);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(IMetaData*, CreateMetaData);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfAttachment
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                   \
  EXPORT(ReturnType, MdfAttachment, FuncName, IAttachment* attachment, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(uint16_t, GetCreatorIndex);
EXPORTFEATUREFUNC(void, SetCreatorIndex, uint16_t index);
EXPORTFEATUREFUNC(bool, GetEmbedded);
EXPORTFEATUREFUNC(void, SetEmbedded, bool embedded);
EXPORTFEATUREFUNC(bool, GetCompressed);
EXPORTFEATUREFUNC(void, SetCompressed, bool compressed);
EXPORTFEATUREFUNC(const char*, GetMd5);
EXPORTFEATUREFUNC(const char*, GetFileName);
EXPORTFEATUREFUNC(void, SetFileName, const char* name);
EXPORTFEATUREFUNC(const char*, GetFileType);
EXPORTFEATUREFUNC(void, SetFileType, const char* type);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFileHistory
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                       \
  EXPORT(ReturnType, MdfFileHistory, FuncName, IFileHistory* file_history, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(uint64_t, GetTime);
EXPORTFEATUREFUNC(void, SetTime, uint64_t time);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(const char*, GetToolName);
EXPORTFEATUREFUNC(void, SetToolName, const char* name);
EXPORTFEATUREFUNC(const char*, GetToolVendor);
EXPORTFEATUREFUNC(void, SetToolVendor, const char* vendor);
EXPORTFEATUREFUNC(const char*, GetToolVersion);
EXPORTFEATUREFUNC(void, SetToolVersion, const char* version);
EXPORTFEATUREFUNC(const char*, GetUserName);
EXPORTFEATUREFUNC(void, SetUserName, const char* user);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfEvent
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfEvent, FuncName, IEvent* event, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(const char*, GetGroupName);
EXPORTFEATUREFUNC(void, SetGroupName, const char* group);
EXPORTFEATUREFUNC(EventType, GetType);
EXPORTFEATUREFUNC(void, SetType, EventType type);
EXPORTFEATUREFUNC(SyncType, GetSync);
EXPORTFEATUREFUNC(void, SetSync, SyncType type);
EXPORTFEATUREFUNC(RangeType, GetRange);
EXPORTFEATUREFUNC(void, SetRange, RangeType type);
EXPORTFEATUREFUNC(EventCause, GetCause);
EXPORTFEATUREFUNC(void, SetCause, EventCause cause);
EXPORTFEATUREFUNC(int64_t, GetCreatorIndex);
EXPORTFEATUREFUNC(void, SetCreatorIndex, int64_t index);
EXPORTFEATUREFUNC(int64_t, GetSyncValue);
EXPORTFEATUREFUNC(void, SetSyncValue, int64_t value);
EXPORTFEATUREFUNC(double, GetSyncFactor);
EXPORTFEATUREFUNC(void, SetSyncFactor, double factor);
EXPORTFEATUREFUNC(const IEvent*, GetParentEvent);
EXPORTFEATUREFUNC(void, SetParentEvent, IEvent* parent);
EXPORTFEATUREFUNC(const IEvent*, GetRangeEvent);
EXPORTFEATUREFUNC(void, SetRangeEvent, IEvent* range);
EXPORTFEATUREFUNC(size_t, GetAttachments, const IAttachment**& pAttachment);
EXPORTFEATUREFUNC(double, GetPreTrig);
EXPORTFEATUREFUNC(void, SetPreTrig, double time);
EXPORTFEATUREFUNC(double, GetPostTrig);
EXPORTFEATUREFUNC(void, SetPostTrig, double time);
EXPORTFEATUREFUNC(const IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(void, AddAttachment, IAttachment* attachment);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfETag
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfETag, FuncName, ETag* etag, ##__VA_ARGS__)
EXPORTFEATUREFUNC(const char*, GetName);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(const char*, GetDescription);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(const char*, GetUnit);
EXPORTFEATUREFUNC(void, SetUnit, const char* unit);
EXPORTFEATUREFUNC(const char*, GetUnitRef);
EXPORTFEATUREFUNC(void, SetUnitRef, const char* unit);
EXPORTFEATUREFUNC(const char*, GetType);
EXPORTFEATUREFUNC(void, SetType, const char* type);
EXPORTFEATUREFUNC(ETagDataType, GetDataType);
EXPORTFEATUREFUNC(void, SetDataType, ETagDataType type);
EXPORTFEATUREFUNC(const char*, GetLanguage);
EXPORTFEATUREFUNC(void, SetLanguage, const char* language);
EXPORTFEATUREFUNC(bool, GetReadOnly);
EXPORTFEATUREFUNC(void, SetReadOnly, bool read_only);
EXPORTFEATUREFUNC(const char*, GetValueAsString);
EXPORTFEATUREFUNC(void, SetValueAsString, const char* value);
EXPORTFEATUREFUNC(double, GetValueAsFloat);
EXPORTFEATUREFUNC(void, SetValueAsFloat, double value);
EXPORTFEATUREFUNC(bool, GetValueAsBoolean);
EXPORTFEATUREFUNC(void, SetValueAsBoolean, bool value);
EXPORTFEATUREFUNC(int64_t, GetValueAsSigned);
EXPORTFEATUREFUNC(void, SetValueAsSigned, int64_t value);
EXPORTFEATUREFUNC(uint64_t, GetValueAsUnsigned);
EXPORTFEATUREFUNC(void, SetValueAsUnsigned, uint64_t value);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfMetaData
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfMetaData, FuncName, IMetaData* metadata, ##__VA_ARGS__)
EXPORTFEATUREFUNC(const char*, GetPropertyAsString, const char* index);
EXPORTFEATUREFUNC(void, SetPropertyAsString, const char* index,
                  const char* prop);
EXPORTFEATUREFUNC(double, GetPropertyAsFloat, const char* index);
EXPORTFEATUREFUNC(void, SetPropertyAsFloat, const char* index, double prop);
EXPORTFEATUREFUNC(size_t, GetProperties, const ETag*& pProperty);
EXPORTFEATUREFUNC(size_t, GetCommonProperties, const ETag*& pProperty);
EXPORTFEATUREFUNC(void, SetCommonProperties, const ETag* pProperty,
                  size_t count);
EXPORTFEATUREFUNC(const char*, GetXmlSnippet);
EXPORTFEATUREFUNC(void, SetXmlSnippet, const char* xml);
EXPORTFEATUREFUNC(void, AddCommonProperty, ETag* tag);
#undef EXPORTFEATUREFUNC
#pragma endregion
}
