/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfAttachment.h"
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfEvent {
 private:
  mdf::IEvent* event;

 public:
  MdfEvent(mdf::IEvent* event) : event(event) {
    if (event == nullptr) throw std::runtime_error("MdfEventInit failed");
  }
  MdfEvent(const mdf::IEvent* event)
      : MdfEvent(const_cast<mdf::IEvent*>(event)) {}
  ~MdfEvent() { event = nullptr; }
  int64_t GetIndex() const { return MdfEventGetIndex(event); }
  std::string GetName() const {
    std::string str;
    str.reserve(MdfEventGetName(event, nullptr) + 1);
    str.resize(MdfEventGetName(event, str.data()));;
    return str;
  }
  void SetName(const char* name) { MdfEventSetName(event, name); }
  std::string GetDescription() const {
    std::string str;
    str.reserve(MdfEventGetDescription(event, nullptr) + 1);
    str.resize(MdfEventGetDescription(event, str.data()));;
    return str;
  }
  void SetDescription(const char* desc) { MdfEventSetDescription(event, desc); }
  std::string GetGroupName() const {
    std::string str;
    str.reserve(MdfEventGetGroupName(event, nullptr) + 1);
    str.resize(MdfEventGetGroupName(event, str.data()));;
    return str;
  }
  void SetGroupName(const char* group) { MdfEventSetGroupName(event, group); }
  EventType GetType() const { return MdfEventGetType(event); }
  void SetType(EventType type) { MdfEventSetType(event, type); }
  SyncType GetSync() const { return MdfEventGetSync(event); }
  void SetSync(SyncType type) { MdfEventSetSync(event, type); }
  RangeType GetRange() const { return MdfEventGetRange(event); }
  void SetRange(RangeType type) { MdfEventSetRange(event, type); }
  EventCause GetCause() const { return MdfEventGetCause(event); }
  void SetCause(EventCause cause) { MdfEventSetCause(event, cause); }
  int64_t GetCreatorIndex() const { return MdfEventGetCreatorIndex(event); }
  void SetCreatorIndex(int64_t index) { MdfEventSetCreatorIndex(event, index); }
  int64_t GetSyncValue() const { return MdfEventGetSyncValue(event); }
  void SetSyncValue(int64_t value) { MdfEventSetSyncValue(event, value); }
  void SetSyncFactor(double factor) { MdfEventSetSyncFactor(event, factor); }
  const MdfEvent GetParentEvent() const {
    return MdfEvent(MdfEventGetParentEvent(event));
  }
  void SetParentEvent(MdfEvent parent) {
    MdfEventSetParentEvent(event, parent.event);
  }
  const MdfEvent GetRangeEvent() const {
    return MdfEvent(MdfEventGetRangeEvent(event));
  }
  void SetRangeEvent(MdfEvent range) {
    MdfEventSetRangeEvent(event, range.event);
  }
  std::vector<MdfAttachment> GetAttachments() const {
    size_t count = MdfEventGetAttachments(event, nullptr);
    if (count <= 0) return std::vector<MdfAttachment>();
    auto pAttachments = new const mdf::IAttachment*[count];
    MdfEventGetAttachments(event, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++)
      attachments.push_back(MdfAttachment(pAttachments[i]));
    delete[] pAttachments;
    return attachments;
  }
  double GetPreTrig() const { return MdfEventGetPreTrig(event); }
  void SetPreTrig(double time) { MdfEventSetPreTrig(event, time); }
  double GetPostTrig() const { return MdfEventGetPostTrig(event); }
  void SetPostTrig(double time) { MdfEventSetPostTrig(event, time); }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfEventGetMetaData(event));
  }
  void AddAttachment(MdfAttachment attachment) {
    MdfEventAddAttachment(event, attachment.GetAttachment());
  }
};
}  // namespace MdfLibrary