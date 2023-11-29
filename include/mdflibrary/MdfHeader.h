/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
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
    size_t size = MdfHeaderGetDescription(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetDescription(header, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDescription(const char* desc) {
    MdfHeaderSetDescription(header, desc);
  }
  std::string GetAuthor() const {
    size_t size = MdfHeaderGetAuthor(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetAuthor(header, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetAuthor(const char* author) { MdfHeaderSetAuthor(header, author); }
  std::string GetDepartment() const {
    size_t size = MdfHeaderGetDepartment(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetDepartment(header, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDepartment(const char* department) {
    MdfHeaderSetDepartment(header, department);
  }
  std::string GetProject() const {
    size_t size = MdfHeaderGetProject(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetProject(header, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetProject(const char* project) { MdfHeaderSetProject(header, project); }
  std::string GetSubject() const {
    size_t size = MdfHeaderGetSubject(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetSubject(header, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetSubject(const char* subject) { MdfHeaderSetSubject(header, subject); }
  std::string GetMeasurementId() const {
    size_t size = MdfHeaderGetMeasurementId(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetMeasurementId(header, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetMeasurementId(const char* uuid) {
    MdfHeaderSetMeasurementId(header, uuid);
  }
  std::string GetRecorderId() const {
    size_t size = MdfHeaderGetRecorderId(header, nullptr);
    char* str = new char[size + 1];
    MdfHeaderGetRecorderId(header, str);
    std::string s(str, size);
    delete str;
    return s;
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
    return MdfHeaderGetMetaDatas(header);
  }
  std::vector<MdfAttachment> GetAttachments() const {
    size_t count = MdfHeaderGetAttachments(header, nullptr);
    if (count <= 0) return {};
    auto pAttachments = new mdf::IAttachment*[count];
    MdfHeaderGetAttachments(header, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++) attachments.push_back(pAttachments[i]);
    delete[] pAttachments;
    return attachments;
  }
  std::vector<MdfFileHistory> GetFileHistorys() const {
    size_t count = MdfHeaderGetFileHistorys(header, nullptr);
    if (count <= 0) return {};
    auto pFileHistorys = new mdf::IFileHistory*[count];
    MdfHeaderGetFileHistorys(header, pFileHistorys);
    std::vector<MdfFileHistory> fileHistorys;
    for (size_t i = 0; i < count; i++) fileHistorys.push_back(pFileHistorys[i]);
    delete[] pFileHistorys;
    return fileHistorys;
  }
  std::vector<MdfEvent> GetEvents() const {
    size_t count = MdfHeaderGetEvents(header, nullptr);
    if (count <= 0) return {};
    auto pEvents = new mdf::IEvent*[count];
    MdfHeaderGetEvents(header, pEvents);
    std::vector<MdfEvent> events;
    for (size_t i = 0; i < count; i++) events.push_back(pEvents[i]);
    delete[] pEvents;
    return events;
  }
  std::vector<MdfDataGroup> GetDataGroups() const {
    size_t count = MdfHeaderGetDataGroups(header, nullptr);
    if (count <= 0) return {};
    auto pDataGroups = new mdf::IDataGroup*[count];
    MdfHeaderGetDataGroups(header, pDataGroups);
    std::vector<MdfDataGroup> dataGroups;
    for (size_t i = 0; i < count; i++) dataGroups.push_back(pDataGroups[i]);
    delete[] pDataGroups;
    return dataGroups;
  }
  MdfAttachment CreateAttachment() { return MdfHeaderCreateAttachment(header); }
  MdfFileHistory CreateFileHistory() {
    return MdfHeaderCreateFileHistory(header);
  }
  MdfEvent CreateEvent() { return MdfHeaderCreateEvent(header); }
  MdfDataGroup CreateDataGroup() { return MdfHeaderCreateDataGroup(header); }
};
}  // namespace MdfLibrary