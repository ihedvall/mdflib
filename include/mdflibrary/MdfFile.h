#pragma once
#include "MdfAttachment.h"
#include "MdfHeader.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfFile {
 private:
  const mdf::MdfFile* file;

 public:
  MdfFile(const mdf::MdfFile* file) : file(file) {}
  ~MdfFile() { file = nullptr; }
  const char* GetName() { return MdfFileGetName(file); }
  void SetName(const char* name) { MdfFileSetName(file, name); }
  const char* GetFileName() { return MdfFileGetFileName(file); }
  void SetFileName(const char* filename) { MdfFileSetFileName(file, filename); }
  const char* GetVersion() { return MdfFileGetVersion(file); }
  int GetMainVersion() { return MdfFileGetMainVersion(file); }
  int GetMinorVersion() { return MdfFileGetMinorVersion(file); }
  void SetMinorVersion(int minor) { MdfFileSetMinorVersion(file, minor); }
  const char* GetProgramId() { return MdfFileGetProgramId(file); }
  void SetProgramId(const char* program_id) {
    MdfFileSetProgramId(file, program_id);
  }
  bool GetFinalized(uint16_t& standard_flags, uint16_t& custom_flags) {
    return MdfFileGetFinalized(file, standard_flags, custom_flags);
  }
  const MdfHeader GetHeader() { return MdfHeader(MdfFileGetHeader(file)); }
  bool GetIsMdf4() { return MdfFileGetIsMdf4(file); }
  std::vector<MdfAttachment> GetAttachments() {
    size_t count = MdfFileGetAttachments(file, nullptr);
    if (count <= 0) return std::vector<MdfAttachment>();
    auto pAttachments = new mdf::IAttachment*[count];
    MdfFileGetAttachments(file, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++)
      attachments.push_back(MdfAttachment(pAttachments[i]));
    delete[] pAttachments;
    return attachments;
  }
  std::vector<MdfDataGroup> GetDataGroups() {
    size_t count = MdfFileGetDataGroups(file, nullptr);
    if (count <= 0) return std::vector<MdfDataGroup>();
    auto pDataGroups = new mdf::IDataGroup*[count];
    MdfFileGetDataGroups(file, pDataGroups);
    std::vector<MdfDataGroup> data_groups;
    for (size_t i = 0; i < count; i++)
      data_groups.push_back(MdfDataGroup(pDataGroups[i]));
    delete[] pDataGroups;
    return data_groups;
  }
  MdfAttachment CreateAttachment() {
    return MdfAttachment(MdfFileCreateAttachment(file));
  }
  MdfDataGroup CreateDataGroup() {
    return MdfDataGroup(MdfFileCreateDataGroup(file));
  }
};
}  // namespace MdfLibrary