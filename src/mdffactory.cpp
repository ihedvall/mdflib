/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdffactory.h"

#include "mdf3file.h"
#include "mdf3writer.h"
#include "mdf4file.h"
#include "mdf4writer.h"
#include "mdf/mdflogstream.h"

using namespace mdf::detail;

namespace mdf {
std::unique_ptr<MdfWriter> MdfFactory::CreateMdfWriter(MdfWriterType type) {
  std::unique_ptr<MdfWriter> writer = std::make_unique<Mdf3Writer>();
  return writer;
}

std::unique_ptr<MdfFile> MdfFactory::CreateMdfFile(MdfFileType type) {
  std::unique_ptr<MdfFile> file = std::make_unique<Mdf3File>();
  return file;
}

MdfWriter* MdfFactory::CreateMdfWriterEx(MdfWriterType type) {
  MdfWriter* writer = nullptr;

  switch (type) {
    case MdfWriterType::Mdf3Basic: {
      writer = new Mdf3Writer();
      break;
    }

    case MdfWriterType::Mdf4Basic: {
      writer = new Mdf4Writer();
      break;
    }

    default:
      break;
  }
  return writer;
}

MdfFile* MdfFactory::CreateMdfFileEx(MdfFileType type) {
  MdfFile* file = nullptr;

  switch (type) {
    case MdfFileType::Mdf3FileType: {
      file = new Mdf3File();
      break;
    }

    case MdfFileType::Mdf4FileType: {
      file = new Mdf4File();
      break;
    }

    default:
      break;
  }
  return file;
}

void MdfFactory::SetLogFunction2(const MdfLogFunction2& func) {
  MdfLogStream::SetLogFunction2(func);
}

}  // namespace mdf
