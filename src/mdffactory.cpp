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
  std::unique_ptr<MdfWriter> writer;

  switch (type) {
    case MdfWriterType::Mdf3Basic: {
      auto temp = std::make_unique<Mdf3Writer>();
      writer = std::move(temp);
      break;
    }

    case MdfWriterType::Mdf4Basic: {
      auto temp = std::make_unique<Mdf4Writer>();
      writer = std::move(temp);
      break;
    }

    default:
      break;
  }
  return writer;
}

std::unique_ptr<MdfFile> MdfFactory::CreateMdfFile(MdfFileType type) {
  std::unique_ptr<MdfFile> file;

  switch (type) {
    case MdfFileType::Mdf3FileType: {
      auto temp = std::make_unique<Mdf3File>();
      file = std::move(temp);
      break;
    }

    case MdfFileType::Mdf4FileType: {
      auto temp = std::make_unique<Mdf4File>();
      file = std::move(temp);
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
