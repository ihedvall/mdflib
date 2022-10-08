/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/mdffile.h"
#include "mdf/mdfwriter.h"

namespace mdf {

enum class MdfWriterType : int { Mdf3Basic = 0, Mdf4Basic = 1 };

enum class MdfFileType : int { Mdf3FileType = 0, Mdf4FileType = 1 };

class MdfFactory {
 public:
  static std::unique_ptr<MdfWriter> CreateMdfWriter(MdfWriterType type);
  static std::unique_ptr<MdfFile> CreateMdfFile(MdfFileType type);
};

}  // namespace mdf