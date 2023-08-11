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
    std::string str;
    size_t size = MdfFileGetName(file, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileGetName(file, str.data());
    return str;
  }
  void SetName(const char* name) { MdfFileSetName(file, name); }
  std::string GetFileName() const {
    std::string str;
    size_t size = MdfFileGetFileName(file, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileGetFileName(file, str.data());
    return str;
  }
  void SetFileName(const char* filename) { MdfFileSetFileName(file, filename); }
  std::string GetVersion() const {
    std::string str;
    size_t size = MdfFileGetVersion(file, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileGetVersion(file, str.data());
    return str;
  }
  int GetMainVersion() const { return MdfFileGetMainVersion(file); }
  int GetMinorVersion() const { return MdfFileGetMinorVersion(file); }
  void SetMinorVersion(int minor) { MdfFileSetMinorVersion(file, minor); }
  std::string GetProgramId() const {
    std::string str;
    size_t size = MdfFileGetProgramId(file, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfFileGetProgramId(file, str.data());
    return str;
  }
  void SetProgramId(const char* program_id) {
    MdfFileSetProgramId(file, program_id);
  }
  bool GetFinalized(uint16_t& standard_flags, uint16_t& custom_flags) {
    return MdfFileGetFinalized(file, standard_flags, custom_flags);
  }
  const MdfHeader GetHeader() const {
    return MdfHeader(MdfFileGetHeader(file));
  }
  bool GetIsMdf4() { return MdfFileGetIsMdf4(file); }
  std::vector<MdfAttachment> GetAttachments() const {
    size_t count = MdfFileGetAttachments(file, nullptr);
    if (count <= 0) return std::vector<MdfAttachment>();
    auto pAttachments = new const mdf::IAttachment*[count];
    MdfFileGetAttachments(file, pAttachments);
    std::vector<MdfAttachment> attachments;
    for (size_t i = 0; i < count; i++)
      attachments.push_back(MdfAttachment(pAttachments[i]));
    delete[] pAttachments;
    return attachments;
  }
  std::vector<MdfDataGroup> GetDataGroups() const {
    size_t count = MdfFileGetDataGroups(file, nullptr);
    if (count <= 0) return std::vector<MdfDataGroup>();
    auto pDataGroups = new const mdf::IDataGroup*[count];
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