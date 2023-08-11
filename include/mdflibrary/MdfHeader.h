#pragma once
#include "MdfDataGroup.h"
#include "MdfEvent.h"
#include "MdfFileHistory.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfHeader {
 private:
  mdf::IHeader* header;

 public:
  MdfHeader(mdf::IHeader* header) : header(header) {
    if (header == nullptr) throw std::runtime_error("MdfHeaderInit failed");
  }
  MdfHeader(const mdf::IHeader* header)
      : MdfHeader(const_cast<mdf::IHeader*>(header)) {}
  ~MdfHeader() { header = nullptr; }
  int64_t GetIndex() const { return MdfHeaderGetIndex(header); }
  std::string GetDescription() const {
    std::string str;
    size_t size = MdfHeaderGetDescription(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetDescription(header, str.data());
    return str;
  }
  void SetDescription(const char* desc) {
    MdfHeaderSetDescription(header, desc);
  }
  std::string GetAuthor() const {
    std::string str;
    size_t size = MdfHeaderGetAuthor(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetAuthor(header, str.data());
    return str;
  }
  void SetAuthor(const char* author) { MdfHeaderSetAuthor(header, author); }
  std::string GetDepartment() const {
    std::string str;
    size_t size = MdfHeaderGetDepartment(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetDepartment(header, str.data());
    return str;
  }
  void SetDepartment(const char* department) {
    MdfHeaderSetDepartment(header, department);
  }
  std::string GetProject() const {
    std::string str;
    size_t size = MdfHeaderGetProject(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetProject(header, str.data());
    return str;
  }
  void SetProject(const char* project) { MdfHeaderSetProject(header, project); }
  std::string GetSubject() const {
    std::string str;
    size_t size = MdfHeaderGetSubject(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetSubject(header, str.data());
    return str;
  }
  void SetSubject(const char* subject) { MdfHeaderSetSubject(header, subject); }
  std::string GetMeasurementId() const {
    std::string str;
    size_t size = MdfHeaderGetMeasurementId(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetMeasurementId(header, str.data());
    return str;
  }
  void SetMeasurementId(const char* uuid) {
    MdfHeaderSetMeasurementId(header, uuid);
  }
  std::string GetRecorderId() const {
    std::string str;
    size_t size = MdfHeaderGetRecorderId(header, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfHeaderGetRecorderId(header, str.data());
    return str;
  }
  void SetRecorderId(const char* uuid) { MdfHeaderSetRecorderId(header, uuid); }
  int64_t GetRecorderIndex() const { return MdfHeaderGetRecorderIndex(header); }
  void SetRecorderIndex(int64_t index) {
    MdfHeaderSetRecorderIndex(header, index);
  }
  uint64_t GetStartTime() const { return MdfHeaderGetStartTime(header); }
  void SetStartTime(uint64_t time) { MdfHeaderSetStartTime(header, time); }
  bool IsStartAngleUsed() { return MdfHeaderIsStartAngleUsed(header); }
  double GetStartAngle() const { return MdfHeaderGetStartAngle(header); }
  void SetStartAngle(double angle) { MdfHeaderSetStartAngle(header, angle); }
  bool IsStartDistanceUsed() { return MdfHeaderIsStartDistanceUsed(header); }
  double GetStartDistance() const { return MdfHeaderGetStartDistance(header); }
  void SetStartDistance(double distance) {
    MdfHeaderSetStartDistance(header, distance);
  }
  const MdfMetaData GetMetaDatas() const {
    return MdfMetaData(MdfHeaderGetMetaDatas(header));
  }
  std::vector<MdfAttachment> GetAttachments() const {
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
  std::vector<MdfFileHistory> GetFileHistorys() const {
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
  std::vector<MdfEvent> GetEvents() const {
    size_t count = MdfHeaderGetEvents(header, nullptr);
    if (count <= 0) return std::vector<MdfEvent>();
    auto pEvents = new mdf::IEvent*[count];
    MdfHeaderGetEvents(header, pEvents);
    std::vector<MdfEvent> events;
    for (size_t i = 0; i < count; i++) events.push_back(MdfEvent(pEvents[i]));
    delete[] pEvents;
    return events;
  }
  std::vector<MdfDataGroup> GetDataGroups() const {
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