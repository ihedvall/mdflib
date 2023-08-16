/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <stdexcept>

#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfAttachment {
 private:
  mdf::IAttachment* attachment;

 public:
  MdfAttachment(mdf::IAttachment* attachment) : attachment(attachment) {
    if (attachment == nullptr)
      throw std::runtime_error("MdfAttachmentInit failed");
  }
  MdfAttachment(const mdf::IAttachment* attachment)
      : MdfAttachment(const_cast<mdf::IAttachment*>(attachment)) {}
  ~MdfAttachment() { attachment = nullptr; }
  mdf::IAttachment* GetAttachment() const { return attachment; }
  int64_t GetIndex() const { return MdfAttachmentGetIndex(attachment); }
  uint16_t GetCreatorIndex() const {
    return MdfAttachmentGetCreatorIndex(attachment);
  }
  void SetCreatorIndex(uint16_t index) {
    MdfAttachmentSetCreatorIndex(attachment, index);
  }
  bool GetEmbedded() const { return MdfAttachmentGetEmbedded(attachment); }
  void SetEmbedded(bool embedded) {
    MdfAttachmentSetEmbedded(attachment, embedded);
  }
  bool GetCompressed() const { return MdfAttachmentGetCompressed(attachment); }
  void SetCompressed(bool compressed) {
    MdfAttachmentSetCompressed(attachment, compressed);
  }
  std::string GetMd5() {
    char str[32 + 1];
    MdfAttachmentGetMd5(attachment, str);
    return std::string(str);
  }
  std::string GetFileName() const {
    std::string str;
    str.reserve(MdfAttachmentGetFileName(attachment, nullptr) + 1);
    str.resize(MdfAttachmentGetFileName(attachment, str.data()));;
    return str;
  }
  void SetFileName(const char* name) {
    MdfAttachmentSetFileName(attachment, name);
  }
  std::string GetFileType() const {
    std::string str;
    str.reserve(MdfAttachmentGetFileType(attachment, nullptr) + 1);
    str.resize(MdfAttachmentGetFileType(attachment, str.data()));;
    return str;
  }
  void SetFileType(const char* type) {
    MdfAttachmentSetFileType(attachment, type);
  }
};
}  // namespace MdfLibrary