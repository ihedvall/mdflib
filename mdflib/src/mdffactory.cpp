/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdffactory.h"

#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"

#include "mdf3file.h"
#include "mdf3writer.h"
#include "mdf4file.h"
#include "mdf4writer.h"
#include "mdfbuslogger.h"
#include "mdfconverter.h"
#include "cg4datawriter.h"

using namespace mdf::detail;

namespace mdf {

std::unique_ptr<MdfReader> MdfFactory::CreateMdfReader(std::string filename) {
  return std::make_unique<MdfReader>(std::move(filename));
}

std::unique_ptr<MdfWriter> MdfFactory::CreateMdfWriter(MdfWriterType type) {
  std::unique_ptr<MdfWriter> writer;

  switch (type) {
    case MdfWriterType::Mdf3Basic: {
      auto mdf3 = std::make_unique<Mdf3Writer>();
      writer = std::move(mdf3);
      break;
    }

    case MdfWriterType::Mdf4Basic: {
      auto mdf4 = std::make_unique<Mdf4Writer>();
      writer = std::move(mdf4);
      break;
    }

    case MdfWriterType::MdfBusLogger: {
      auto mdf4 = std::make_unique<MdfBusLogger>();
      writer = std::move(mdf4);
      break;
    }

    case MdfWriterType::MdfConverter: {
      auto mdf4 = std::make_unique<MdfConverter>();
      writer = std::move(mdf4);
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
      auto mdf3 = std::make_unique<Mdf3File>();
      file = std::move(mdf3);
      break;
    }
    case MdfFileType::Mdf4FileType: {
      auto mdf4 = std::make_unique<Mdf4File>();
      file = std::move(mdf4);
      break;
    }

    default:
      break;
  }
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
    
    case MdfWriterType::MdfBusLogger: {
      writer = new MdfBusLogger();
      break;
    }

    case MdfWriterType::MdfConverter: {
      writer = new MdfConverter();
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

std::unique_ptr<IChannelObserver> MdfFactory::CreateChannelObserver(
    const IDataGroup& data_group, const IChannelGroup& group,
    const IChannel& channel) {
  return mdf::CreateChannelObserver(data_group, group, channel);
}

std::unique_ptr<IDataWriter> MdfFactory::CreateDataWriter(
    IChannelGroup& channel_group, MdfFileType type) {
  std::unique_ptr<IDataWriter> writer;
  switch (type) {
    case MdfFileType::Mdf4FileType: {
      writer = std::make_unique<Cg4DataWriter>(channel_group);
      break;
    }

    default:
      break;
  }
  return writer;
}

}  // namespace mdf
