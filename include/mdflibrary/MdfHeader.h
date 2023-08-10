#pragma once
#include "MdfDataGroup.h"
#include "MdfEvent.h"
#include "MdfFileHistory.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfHeader {
 private:
  const mdf::IHeader* header;

 public:
  MdfHeader(const mdf::IHeader* header) : header(header) {}
  ~MdfHeader() { header = nullptr; }
  int64_t GetIndex() { return MdfHeaderGetIndex(header); }
  const char* GetDescription() { return MdfHeaderGetDescription(header); }
  void SetDescription(const char* desc) {
    MdfHeaderSetDescription(header, desc);
  }
  const char* GetAuthor() { return MdfHeaderGetAuthor(header); }
  void SetAuthor(const char* author) { MdfHeaderSetAuthor(header, author); }
  const char* GetDepartment() { return MdfHeaderGetDepartment(header); }
  void SetDepartment(const char* department) {
    MdfHeaderSetDepartment(header, department);
  }
  const char* GetProject() { return MdfHeaderGetProject(header); }
  void SetProject(const char* project) { MdfHeaderSetProject(header, project); }
  const char* GetSubject() { return MdfHeaderGetSubject(header); }
  void SetSubject(const char* subject) { MdfHeaderSetSubject(header, subject); }
  const char* GetMeasurementId() { return MdfHeaderGetMeasurementId(header); }
  void SetMeasurementId(const char* uuid) {
    MdfHeaderSetMeasurementId(header, uuid);
  }
  const char* GetRecorderId() { return MdfHeaderGetRecorderId(header); }
  void SetRecorderId(const char* uuid) { MdfHeaderSetRecorderId(header, uuid); }
  int64_t GetRecorderIndex() { return MdfHeaderGetRecorderIndex(header); }
  void SetRecorderIndex(int64_t index) {
    MdfHeaderSetRecorderIndex(header, index);
  }
  uint64_t GetStartTime() { return MdfHeaderGetStartTime(header); }
  void SetStartTime(uint64_t time) { MdfHeaderSetStartTime(header, time); }
  bool IsStartAngleUsed() { return MdfHeaderIsStartAngleUsed(header); }
  double GetStartAngle() { return MdfHeaderGetStartAngle(header); }
  void SetStartAngle(double angle) { MdfHeaderSetStartAngle(header, angle); }
  bool IsStartDistanceUsed() { return MdfHeaderIsStartDistanceUsed(header); }
  double GetStartDistance() { return MdfHeaderGetStartDistance(header); }
  void SetStartDistance(double distance) {
    MdfHeaderSetStartDistance(header, distance);
  }
  const MdfMetaData GetMetaDatas() {
    return MdfMetaData(MdfHeaderGetMetaDatas(header));
  }
  std::vector<MdfAttachment> GetAttachments() {
    size_t count = MdfHeaderGetAttachments(header, nullptr);
    if (count <= 0) return std::vector<MdfAttachment>();
    auto pAttachments = new mdf::IAttachment*[count];
    MdfHeaderGetAttachments(header, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++)
      attachments.push_back(MdfAttachment(pAttachments[i]));
    delete[] pAttachments;
    return attachments;
  }
  std::vector<MdfFileHistory> GetFileHistorys() {
    size_t count = MdfHeaderGetFileHistorys(header, nullptr);
    if (count <= 0) return std::vector<MdfFileHistory>();
    auto pFileHistorys = new mdf::IFileHistory*[count];
    MdfHeaderGetFileHistorys(header, pFileHistorys);
    std::vector<MdfFileHistory> fileHistorys;
    for (size_t i = 0; i < count; i++)
      fileHistorys.push_back(MdfFileHistory(pFileHistorys[i]));
    delete[] pFileHistorys;
    return fileHistorys;
  }
  std::vector<MdfEvent> GetEvents() {
    size_t count = MdfHeaderGetEvents(header, nullptr);
    if (count <= 0) return std::vector<MdfEvent>();
    auto pEvents = new mdf::IEvent*[count];
    MdfHeaderGetEvents(header, pEvents);
    std::vector<MdfEvent> events;
    for (size_t i = 0; i < count; i++) events.push_back(MdfEvent(pEvents[i]));
    delete[] pEvents;
    return events;
  }
  std::vector<MdfDataGroup> GetDataGroups() {
    size_t count = MdfHeaderGetDataGroups(header, nullptr);
    if (count <= 0) return std::vector<MdfDataGroup>();
    auto pDataGroups = new mdf::IDataGroup*[count];
    MdfHeaderGetDataGroups(header, pDataGroups);
    std::vector<MdfDataGroup> dataGroups;
    for (size_t i = 0; i < count; i++)
      dataGroups.push_back(MdfDataGroup(pDataGroups[i]));
    delete[] pDataGroups;
    return dataGroups;
  }
  MdfAttachment CreateAttachment() {
    return MdfAttachment(MdfHeaderCreateAttachment(header));
  }
  MdfFileHistory CreateFileHistory() {
    return MdfFileHistory(MdfHeaderCreateFileHistory(header));
  }
  MdfEvent CreateEvent() { return MdfEvent(MdfHeaderCreateEvent(header)); }
  MdfDataGroup CreateDataGroup() {
    return MdfDataGroup(MdfHeaderCreateDataGroup(header));
  }
};
}  // namespace MdfLibrary