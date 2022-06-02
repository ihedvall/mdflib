/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <optional>
#include <vector>

namespace mdf {

class IAttachment
{
 public:

  virtual void CreatorIndex(uint16_t creator) = 0;
  [[nodiscard]] virtual uint16_t CreatorIndex() const = 0;

  virtual void IsEmbedded(bool embed) = 0;
  [[nodiscard]] virtual  bool IsEmbedded() const = 0;

  virtual void IsCompressed(bool compress) = 0;
  virtual bool IsCompressed() const = 0;

  [[nodiscard]] virtual std::optional<std::string> Md5() = 0;

  virtual void FileName(const std::string& filename) = 0;
  [[nodiscard]] virtual const std::string& FileName() const = 0;

  virtual void FileType(const std::string& file_type) = 0;
  virtual const std::string& FileType() const = 0;
};

} // namespace mdf
