#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfFileHistory {
 private:
  mdf::IFileHistory* history;

 public:
  MdfFileHistory(mdf::IFileHistory* history) : history(history) {
    if (history == nullptr)
      throw std::runtime_error("MdfFileHistoryInit failed");
  }
  MdfFileHistory(const mdf::IFileHistory* history)
      : MdfFileHistory(const_cast<mdf::IFileHistory*>(history)) {}
  ~MdfFileHistory() { history = nullptr; }
  int64_t GetIndex() const { return MdfFileHistoryGetIndex(history); }
  uint64_t GetTime() const { return MdfFileHistoryGetTime(history); }
  void SetTime(uint64_t time) { MdfFileHistorySetTime(history, time); }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfFileHistoryGetMetaData(history));
  }
  std::string GetDescription() const {
    std::string str;
    size_t size = MdfFileHistoryGetDescription(history, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileHistoryGetDescription(history, str.data());
    return str;
  }
  void SetDescription(const char* desc) {
    MdfFileHistorySetDescription(history, desc);
  }
  std::string GetToolName() const {
    std::string str;
    size_t size = MdfFileHistoryGetToolName(history, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileHistoryGetToolName(history, str.data());
    return str;
  }
  void SetToolName(const char* name) {
    MdfFileHistorySetToolName(history, name);
  }
  std::string GetToolVendor() const {
    std::string str;
    size_t size = MdfFileHistoryGetToolVendor(history, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileHistoryGetToolVendor(history, str.data());
    return str;
  }
  void SetToolVendor(const char* vendor) {
    MdfFileHistorySetToolVendor(history, vendor);
  }
  std::string GetToolVersion() const {
    std::string str;
    size_t size = MdfFileHistoryGetToolVersion(history, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileHistoryGetToolVersion(history, str.data());
    return str;
  }
  void SetToolVersion(const char* version) {
    MdfFileHistorySetToolVersion(history, version);
  }
  std::string GetUserName() const {
    std::string str;
    size_t size = MdfFileHistoryGetUserName(history, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileHistoryGetUserName(history, str.data());
    return str;
  }
  void SetUserName(const char* user) {
    MdfFileHistorySetUserName(history, user);
  }
};
}  // namespace MdfLibrary
