/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/mdfreader.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "channelobserver.h"
#include "dg4block.h"
#include "idblock.h"
#include "mdf/mdflogstream.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "platform.h"
#include "cn4block.h"
#include "sr4block.h"
#include "sr3block.h"
#include "fastchannelobserver.h"
#include <iostream>
using namespace std::chrono_literals;

namespace mdf {

bool IsMdfFile(const std::string &filename) {
  FILE* file = nullptr;
  Platform::fileopen(&file, filename.c_str(), "rb");
  if (file == nullptr) {
    return false;
  }

  detail::IdBlock oId;
  bool bError = true;
  try {
    size_t bytes = oId.Read(file);
    if (bytes == 64) {
      bError = false;
    }
  } catch (const std::exception &) {
    bError = true;
  }
  std::fclose(file);
  if (bError) {
    return false;
  }
  if (Platform::strnicmp(oId.FileId().c_str(), "MDF", 3) == 0 ||
      Platform::strnicmp(oId.FileId().c_str(), "UnFinMF", 7) == 0) {
    return true;
  }

  return false;
}

/// Creates a channel sample observer. The sample observer creates internal
/// memory for all samples.\n The function also attach the observer to the
/// notifier (see observer pattern). The destructor of the channel observer
/// detach the observer. \param data_group \param group \param channel \return
/// Smart pointer to a channel observer
ChannelObserverPtr CreateChannelObserver(const IDataGroup &data_group,
                                         const IChannelGroup &group,
                                         const IChannel &channel) {
  std::unique_ptr<IChannelObserver> observer;
  const auto* channel_array = channel.ChannelArray();
  // Note, that num,ber of bytes returns number of bytes for
  // all value in the array
  const auto array_size = channel_array != nullptr ? channel_array->NofArrayValues() : 1;
  const auto value_size = channel.DataBytes() / array_size;

  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
      switch (value_size) {
        case 1:
          observer = std::make_unique<detail::ChannelObserver<uint8_t>>(
              data_group, group, channel);
          break;

          case 2:
          observer = std::make_unique<detail::ChannelObserver<uint16_t>>(
              data_group, group, channel);
          break;

          case 4:
          observer = std::make_unique<detail::ChannelObserver<uint32_t>>(
              data_group, group, channel);
          break;

          case 8:
          default:
          observer = std::make_unique<detail::ChannelObserver<uint64_t>>(
              data_group, group, channel);
          break;
      }
      break;

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
      if (value_size <= 1) {
        observer = std::make_unique<detail::ChannelObserver<int8_t>>(
            data_group, group, channel);
      } else if (value_size <= 2) {
        observer = std::make_unique<detail::ChannelObserver<int16_t>>(
            data_group, group, channel);
      } else if (value_size <= 4) {
        observer = std::make_unique<detail::ChannelObserver<int32_t>>(
            data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<int64_t>>(
            data_group, group, channel);
      }
      break;

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
      if (value_size <= 4) {
        observer = std::make_unique<detail::ChannelObserver<float>>(
            data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<double>>(
            data_group, group, channel);
      }
      break;

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
      observer = std::make_unique<detail::ChannelObserver<std::string>>(
          data_group, group, channel);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      observer =
          std::make_unique<detail::ChannelObserver<std::vector<uint8_t>>>(
              data_group, group, channel);
      break;

    case ChannelDataType::CanOpenDate:  // Convert to ms since 1970
    case ChannelDataType::CanOpenTime:
      observer = std::make_unique<detail::ChannelObserver<uint64_t>>(
          data_group, group, channel);
      break;

    default:
      break;
  }
  return std::move(observer);
}

ChannelObserverPtr CreateChannelObserver(const IDataGroup &dg_group,
                                         const std::string &channel_name) {
  std::unique_ptr<IChannelObserver> observer;

  const IChannelGroup *channel_group = nullptr;
  const IChannel *channel = nullptr;
  uint64_t nof_samples = 0;
  const auto cg_list = dg_group.ChannelGroups();
  for (const auto *cg_group : cg_list) {
    if (cg_group == nullptr) {
      continue;
    }
    const auto cn_list = cg_group->Channels();
    for (const auto *cn_item : cn_list) {
      if (cn_item == nullptr) {
        continue;
      }
      if (Platform::stricmp(channel_name.c_str(), cn_item->Name().c_str()) ==
          0) {
        if (nof_samples <= cg_group->NofSamples()) {
          nof_samples = cg_group->NofSamples();
          channel_group = cg_group;
          channel = cn_item;
        }
      }
    }
  }
  if (channel_group != nullptr && channel != nullptr) {
    observer = CreateChannelObserver(dg_group, *channel_group, *channel);
  }
  return observer;
}

void CreateChannelObserverForChannelGroup(const IDataGroup &data_group,
                                          const IChannelGroup &group,
                                          ChannelObserverList &dest) {
  auto cn_list = group.Channels();
  for (const auto *cn : cn_list) {
    if (cn != nullptr) {
      dest.emplace_back(
          std::move(CreateChannelObserver(data_group, group, *cn)));
    }
  }
}
ChannelObserverPtr CreateFastChannelObserver(
    const IDataGroup& data_group, const IChannelGroup& group,
    const IChannel& channel){
  std::unique_ptr<IChannelObserver> observer;
  const auto* channel_array = channel.ChannelArray();
  // Note, that num,ber of bytes returns number of bytes for
  // all value in the array
  const auto array_size = channel_array != nullptr ? channel_array->NofArrayValues() : 1;
  const auto value_size = channel.DataBytes() / array_size;

  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
      switch (value_size) {
        case 1:
          observer = std::make_unique<detail::FastChannelObserver<uint8_t>>(
              data_group, group, channel);
          break;

        case 2:
          observer = std::make_unique<detail::FastChannelObserver<uint16_t>>(
              data_group, group, channel);
          break;

        case 4:
          observer = std::make_unique<detail::ChannelObserver<uint32_t>>(
              data_group, group, channel);
          break;

        case 8:
        default:
          observer = std::make_unique<detail::FastChannelObserver<uint64_t>>(
              data_group, group, channel);
          break;
      }
      break;

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
      if (value_size <= 1) {
        observer = std::make_unique<detail::FastChannelObserver<int8_t>>(
            data_group, group, channel);
      } else if (value_size <= 2) {
        observer = std::make_unique<detail::FastChannelObserver<int16_t>>(
            data_group, group, channel);
      } else if (value_size <= 4) {
        observer = std::make_unique<detail::FastChannelObserver<int32_t>>(
            data_group, group, channel);
      } else {
        observer = std::make_unique<detail::FastChannelObserver<int64_t>>(
            data_group, group, channel);
      }
      break;

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
      if (value_size <= 4) {
        observer = std::make_unique<detail::FastChannelObserver<float>>(
            data_group, group, channel);
      } else {
        observer = std::make_unique<detail::FastChannelObserver<double>>(
            data_group, group, channel);
      }
      break;

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
      observer = std::make_unique<detail::FastChannelObserver<std::string>>(
          data_group, group, channel);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      observer =
          std::make_unique<detail::FastChannelObserver<std::vector<uint8_t>>>(
              data_group, group, channel);
      break;

    case ChannelDataType::CanOpenDate:  // Convert to ms since 1970
    case ChannelDataType::CanOpenTime:
      observer = std::make_unique<detail::FastChannelObserver<uint64_t>>(
          data_group, group, channel);
      break;

    default:
      break;
  }
  return std::move(observer);
}

ChannelObserverPtr CreateFastChannelObserver(
    const IDataGroup& dg_group, const std::string& channel_name){
  std::unique_ptr<IChannelObserver> observer;

  const IChannelGroup *channel_group = nullptr;
  const IChannel *channel = nullptr;
  uint64_t nof_samples = 0;
  const auto cg_list = dg_group.ChannelGroups();
  for (const auto *cg_group : cg_list) {
    if (cg_group == nullptr) {
      continue;
    }
    const auto cn_list = cg_group->Channels();
    for (const auto *cn_item : cn_list) {
      if (cn_item == nullptr) {
        continue;
      }
      if (Platform::stricmp(channel_name.c_str(), cn_item->Name().c_str()) ==
          0) {
        if (nof_samples <= cg_group->NofSamples()) {
          nof_samples = cg_group->NofSamples();
          channel_group = cg_group;
          channel = cn_item;
        }
      }
    }
  }
  if (channel_group != nullptr && channel != nullptr) {
    observer = CreateFastChannelObserver(dg_group, *channel_group, *channel);
  }
  return observer;
}
void CreateFastChannelObserverForChannelGroup(const IDataGroup& data_group,
                                              const IChannelGroup& group,
                                              ChannelObserverList& dest){
  auto cn_list = group.Channels();
  for (const auto *cn : cn_list) {
    if (cn != nullptr) {
      dest.emplace_back(
          std::move(CreateFastChannelObserver(data_group, group, *cn)));
    }
  }

}
MdfReader::MdfReader(const std::string &filename) : filename_(filename) {
  // Need to create MDF3 of MDF4 file
  bool bExist = false;
  try {
    std::filesystem::path p = std::filesystem::u8path(filename_);
    if (std::filesystem::exists(p)) {
      bExist = true;
    }
  } catch (const std::exception &error) {
    MDF_ERROR() << "File I/O error. Filename: " << filename_
                << ", Error: " << error.what();
    return;
  }

  if (!bExist) {
    MDF_ERROR() << "The file doesn't exist. Filename: " << filename_;
    // No meaning to continue if the file doesn't exist
    return;
  }
  bool open = Open();
  if (!open ) {
    MDF_ERROR()
        << "The file couldn't be opened for reading (locked?). Filename: "
        << filename;
    // No meaning to continue if the file doesn't exist
    return;
  }
  std::unique_ptr<detail::IdBlock> id_block =
      std::make_unique<detail::IdBlock>();
  id_block->Read(file_);

  if (Platform::strnicmp(id_block->FileId().c_str(), "MDF", 3) == 0 ||
      Platform::strnicmp(id_block->FileId().c_str(), "UnFinMF", 7) == 0) {
    if (id_block->Version() >= 400) {
      instance_ = std::make_unique<detail::Mdf4File>(std::move(id_block));
      instance_->FileName(filename_);
    } else {
      instance_ = std::make_unique<detail::Mdf3File>(std::move(id_block));
      instance_->FileName(filename_);
    }
    if (!instance_) {
      Close();
      MDF_ERROR() << "MDF version not supported. File: " << filename_;
    }
  } else {
    MDF_ERROR() << "This is not and MDF file. File: " << filename_;
    Close();
  }
}

MdfReader::~MdfReader() { Close(); }

std::string MdfReader::ShortName() const {
  try {
    auto filename = std::filesystem::u8path(filename_).stem().u8string();
    return std::string(filename.begin(), filename.end());
  } catch (const std::exception &) {
  }
  return {};
}

bool MdfReader::Open() { return detail::OpenMdfFile(file_, filename_, "rb"); }

void MdfReader::Close() {
  if (file_ != nullptr) {
    fclose(file_);
    file_ = nullptr;
  }
}

bool MdfReader::ReadHeader() {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  // If file is not open then open and close the file in this call
  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "File is not open. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    instance_->ReadHeader(file_);
  } catch (const std::exception &error) {
    MDF_ERROR() << "Initialization failed. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadMeasurementInfo() {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadMeasurementInfo(file_);

  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the DG/CG blocks. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadEverythingButData() {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadEverythingButData(file_);

  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ExportAttachmentData(const IAttachment &attachment,
                                     const std::string &dest_file) {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    auto &at4 = dynamic_cast<const detail::At4Block &>(attachment);
    at4.ReadData(file_, dest_file);
  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadData(IDataGroup &data_group) {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    if (instance_->IsMdf4()) {
      auto &dg4 = dynamic_cast<detail::Dg4Block &>(data_group);
      dg4.ReadData(file_);
    } else {
      auto &dg3 = dynamic_cast<detail::Dg3Block &>(data_group);
      dg3.ReadData(file_);
    }
  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadSrData(ISampleReduction &sr_group) {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    if (instance_->IsMdf4()) {
      auto &sr4 = dynamic_cast<detail::Sr4Block &>(sr_group);
      sr4.ReadData(file_);
    } else {
      auto &sr3 = dynamic_cast<detail::Sr3Block &>(sr_group);
      sr3.ReadData(file_);
    }
  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

const IHeader *MdfReader::GetHeader() const {
  const auto *file = GetFile();
  return file != nullptr ? file->Header() : nullptr;
}

const IDataGroup *MdfReader::GetDataGroup(size_t order) const {
  const auto *file = GetFile();
  if (file != nullptr) {
    DataGroupList dg_list;
    file->DataGroups(dg_list);
    if (order < dg_list.size()) {
      return dg_list[order];
    }
  }
  return nullptr;
}
bool MdfReader::ReadOneData(mdf::IDataGroup &data_group) {
  if (!instance_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    if (instance_->IsMdf4()) {
      MDF_ERROR() << "Read dg4 " << filename_;
      auto &dg4 = dynamic_cast<detail::Dg4Block &>(data_group);
      no_error = dg4.ReadOneData(file_);
    } else {
      /*
      auto &dg3 = dynamic_cast<detail::Dg3Block &>(data_group);
      bool result = dg3.ReadData(file_);
       */
      return false;
    }
  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    std::cout<<"error:"<<error.what()<<std::endl;
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}
}  // namespace mdf

