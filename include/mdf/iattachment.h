/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>

namespace mdf {

class IAttachment
{
 public:
  virtual bool IsEmbedded() const = 0;
  virtual bool IsCompressed() const = 0;
  virtual std::string FileName() const = 0;
  virtual std::string FileType() const = 0;
};

} // namespace mdf
