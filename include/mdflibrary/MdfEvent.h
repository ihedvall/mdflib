#pragma once
#include "MdfAttachment.h"
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfEvent {
 private:
  const mdf::IEvent* event;

 public:
  MdfEvent(const mdf::IEvent* event) : event(event) {}
  ~MdfEvent() { event = nullptr; }
  int64_t GetIndex() { return MdfEventGetIndex(event); }
  const char* GetName() { return MdfEventGetName(event); }
  void SetName(const char* name) { MdfEventSetName(event, name); }
  const char* GetDescription() { return MdfEventGetDescription(event); }
  void SetDescription(const char* desc) { MdfEventSetDescription(event, desc); }
  const char* GetGroupName() { return MdfEventGetGroupName(event); }
  void SetGroupName(const char* group) { MdfEventSetGroupName(event, group); }
  EventType GetType() { return MdfEventGetType(event); }
  void SetType(EventType type) { MdfEventSetType(event, type); }
  SyncType GetSync() { return MdfEventGetSync(event); }
  void SetSync(SyncType type) { MdfEventSetSync(event, type); }
  RangeType GetRange() { return MdfEventGetRange(event); }
  void SetRange(RangeType type) { MdfEventSetRange(event, type); }
  EventCause GetCause() { return MdfEventGetCause(event); }
  void SetCause(EventCause cause) { MdfEventSetCause(event, cause); }
  int64_t GetCreatorIndex() { return MdfEventGetCreatorIndex(event); }
  void SetCreatorIndex(int64_t index) { MdfEventSetCreatorIndex(event, index); }
  int64_t GetSyncValue() { return MdfEventGetSyncValue(event); }
  void SetSyncValue(int64_t value) { MdfEventSetSyncValue(event, value); }
  void SetSyncFactor(double factor) { MdfEventSetSyncFactor(event, factor); }
  const MdfEvent GetParentEvent() {
    return MdfEvent(MdfEventGetParentEvent(event));
  }
  void SetParentEvent(MdfEvent parent) {
    MdfEventSetParentEvent(event, parent.event);
  }
  const MdfEvent GetRangeEvent() {
    return MdfEvent(MdfEventGetRangeEvent(event));
  }
  void SetRangeEvent(MdfEvent range) {
    MdfEventSetRangeEvent(event, range.event);
  }
  std::vector<MdfAttachment> GetAttachments() {
    size_t count = MdfEventGetAttachments(event, nullptr);
    if (count <= 0) return std::vector<MdfAttachment>();
    auto pAttachments = new mdf::IAttachment*[count];
    MdfEventGetAttachments(event, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++)
      attachments.push_back(MdfAttachment(pAttachments[i]));
    return attachments;
  }
  double GetPreTrig() { return MdfEventGetPreTrig(event); }
  void SetPreTrig(double time) { MdfEventSetPreTrig(event, time); }
  double GetPostTrig() { return MdfEventGetPostTrig(event); }
  void SetPostTrig(double time) { MdfEventSetPostTrig(event, time); }
  const MdfMetaData GetMetaData() {
    return MdfMetaData(MdfEventGetMetaData(event));
  }
  void AddAttachment(MdfAttachment attachment) {
    MdfEventAddAttachment(event, attachment.GetAttachment());
  }
};
}  // namespace MdfLibrary