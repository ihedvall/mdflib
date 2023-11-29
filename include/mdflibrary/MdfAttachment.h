/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdexcept>
#include <string>

#include "MdfMetaData.h"

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
    size_t size = MdfAttachmentGetFileName(attachment, nullptr);
    char* str = new char[size + 1];
    MdfAttachmentGetFileName(attachment, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetFileName(const char* name) {
    MdfAttachmentSetFileName(attachment, name);
  }
  std::string GetFileType() const {
    size_t size = MdfAttachmentGetFileType(attachment, nullptr);
    char* str = new char[size + 1];
    MdfAttachmentGetFileType(attachment, str);
    std::string s(str, size);
    delete str;
    return s;
  }
  void SetFileType(const char* type) {
    MdfAttachmentSetFileType(attachment, type);
  }
  const MdfMetaData GetMetaData() const {
    return MdfAttachmentGetMetaData(attachment);
  }
  MdfMetaData CreateMetaData() {
    return MdfAttachmentCreateMetaData(attachment);
  }
};
}  // namespace MdfLibrary