/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfAttachment.h"
#include "MdfHeader.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfFile {
 private:
  mdf::MdfFile* file;

 public:
  MdfFile(mdf::MdfFile* file) : file(file) {
    if (file == nullptr) throw std::runtime_error("MdfFileInit failed");
  }
  MdfFile(const mdf::MdfFile* file)
      : MdfFile(const_cast<mdf::MdfFile*>(file)) {}
  ~MdfFile() { file = nullptr; }
  std::string GetName() const {
    size_t size = MdfFileGetName(file, nullptr);
    char* str = new char[size + 1];
    MdfFileGetName(file, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetName(const char* name) { MdfFileSetName(file, name); }
  std::string GetFileName() const {
    size_t size = MdfFileGetFileName(file, nullptr);
    char* str = new char[size + 1];
    MdfFileGetFileName(file, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetFileName(const char* filename) { MdfFileSetFileName(file, filename); }
  std::string GetVersion() const {
    size_t size = MdfFileGetVersion(file, nullptr);
    char* str = new char[size + 1];
    MdfFileGetVersion(file, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  int GetMainVersion() const { return MdfFileGetMainVersion(file); }
  int GetMinorVersion() const { return MdfFileGetMinorVersion(file); }
  void SetMinorVersion(int minor) { MdfFileSetMinorVersion(file, minor); }
  std::string GetProgramId() const {
    size_t size = MdfFileGetProgramId(file, nullptr);
    char* str = new char[size + 1];
    MdfFileGetProgramId(file, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetProgramId(const char* program_id) {
    MdfFileSetProgramId(file, program_id);
  }
  bool GetFinalized(uint16_t& standard_flags, uint16_t& custom_flags) {
    return MdfFileGetFinalized(file, standard_flags, custom_flags);
  }
  const MdfHeader GetHeader() const { return MdfFileGetHeader(file); }
  bool GetIsMdf4() { return MdfFileGetIsMdf4(file); }
  std::vector<MdfAttachment> GetAttachments() const {
    size_t count = MdfFileGetAttachments(file, nullptr);
    if (count <= 0) return {};
    auto pAttachments = new const mdf::IAttachment*[count];
    MdfFileGetAttachments(file, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++) attachments.push_back(pAttachments[i]);
    delete[] pAttachments;
    return attachments;
  }
  std::vector<MdfDataGroup> GetDataGroups() const {
    size_t count = MdfFileGetDataGroups(file, nullptr);
    if (count <= 0) return {};
    auto pDataGroups = new const mdf::IDataGroup*[count];
    MdfFileGetDataGroups(file, pDataGroups);
    std::vector<MdfDataGroup> data_groups;
    for (size_t i = 0; i < count; i++) data_groups.push_back(pDataGroups[i]);
    delete[] pDataGroups;
    return data_groups;
  }
  MdfAttachment CreateAttachment() { return MdfFileCreateAttachment(file); }
  MdfDataGroup CreateDataGroup() { return MdfFileCreateDataGroup(file); }
};
}  // namespace MdfLibrary