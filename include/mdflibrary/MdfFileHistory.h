#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfFileHistory {
 private:
  mdf::IFileHistory* history;

 public:
  MdfFileHistory(mdf::IFileHistory* history) { this->history = history; }
  ~MdfFileHistory() { this->history = nullptr; }
  int64_t GetIndex() { return MdfFileHistoryGetIndex(history); }
  uint64_t GetTime() { return MdfFileHistoryGetTime(history); }
  void SetTime(uint64_t time) { MdfFileHistorySetTime(history, time); }
  const MdfMetaData GetMetaData() {
    return MdfMetaData(MdfFileHistoryGetMetaData(history));
  }
  const char* GetDescription() { return MdfFileHistoryGetDescription(history); }
  void SetDescription(const char* desc) {
    MdfFileHistorySetDescription(history, desc);
  }
  const char* GetToolName() { return MdfFileHistoryGetToolName(history); }
  void SetToolName(const char* name) {
    MdfFileHistorySetToolName(history, name);
  }
  const char* GetToolVendor() { return MdfFileHistoryGetToolVendor(history); }
  void SetToolVendor(const char* vendor) {
    MdfFileHistorySetToolVendor(history, vendor);
  }
  const char* GetToolVersion() { return MdfFileHistoryGetToolVersion(history); }
  void SetToolVersion(const char* version) {
    MdfFileHistorySetToolVersion(history, version);
  }
  const char* GetUserName() { return MdfFileHistoryGetUserName(history); }
  void SetUserName(const char* user) {
    MdfFileHistorySetUserName(history, user);
  }
};
}  // namespace MdfLibrary
