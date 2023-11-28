/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
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
    size_t size = MdfFileHistoryGetDescription(history, nullptr);
    char* str = new char[size + 1];
    MdfFileHistoryGetDescription(history, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetDescription(const char* desc) {
    MdfFileHistorySetDescription(history, desc);
  }
  std::string GetToolName() const {
    size_t size = MdfFileHistoryGetToolName(history, nullptr);
    char* str = new char[size + 1];
    MdfFileHistoryGetToolName(history, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetToolName(const char* name) {
    MdfFileHistorySetToolName(history, name);
  }
  std::string GetToolVendor() const {
    size_t size = MdfFileHistoryGetToolVendor(history, nullptr);
    char* str = new char[size + 1];
    MdfFileHistoryGetToolVendor(history, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetToolVendor(const char* vendor) {
    MdfFileHistorySetToolVendor(history, vendor);
  }
  std::string GetToolVersion() const {
    size_t size = MdfFileHistoryGetToolVersion(history, nullptr);
    char* str = new char[size + 1];
    MdfFileHistoryGetToolVersion(history, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetToolVersion(const char* version) {
    MdfFileHistorySetToolVersion(history, version);
  }
  std::string GetUserName() const {
    size_t size = MdfFileHistoryGetUserName(history, nullptr);
    char* str = new char[size + 1];
    MdfFileHistoryGetUserName(history, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetUserName(const char* user) {
    MdfFileHistorySetUserName(history, user);
  }
};
}  // namespace MdfLibrary
