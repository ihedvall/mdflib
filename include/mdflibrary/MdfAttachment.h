#pragma once
#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfAttachment {
 private:
  const mdf::IAttachment* attachment;

 public:
  MdfAttachment(const mdf::IAttachment* attachment) : attachment(attachment) {}
  ~MdfAttachment() { attachment = nullptr; }
  const mdf::IAttachment* GetAttachment() { return attachment; }
  int64_t GetIndex() { return MdfAttachmentGetIndex(attachment); }
  uint16_t GetCreatorIndex() {
    return MdfAttachmentGetCreatorIndex(attachment);
  }
  void SetCreatorIndex(uint16_t index) {
    MdfAttachmentSetCreatorIndex(attachment, index);
  }
  bool GetEmbedded() { return MdfAttachmentGetEmbedded(attachment); }
  void SetEmbedded(bool embedded) {
    MdfAttachmentSetEmbedded(attachment, embedded);
  }
  bool GetCompressed() { return MdfAttachmentGetCompressed(attachment); }
  void SetCompressed(bool compressed) {
    MdfAttachmentSetCompressed(attachment, compressed);
  }
  const char* GetMd5() { return MdfAttachmentGetMd5(attachment); }
  const char* GetFileName() { return MdfAttachmentGetFileName(attachment); }
  void SetFileName(const char* name) {
    MdfAttachmentSetFileName(attachment, name);
  }
  const char* GetFileType() { return MdfAttachmentGetFileType(attachment); }
  void SetFileType(const char* type) {
    MdfAttachmentSetFileType(attachment, type);
  }
};
}  // namespace MdfLibrary