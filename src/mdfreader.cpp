/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <cstdio>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>

#include "util/stringutil.h"
#include "util/logstream.h"
#include "mdf/mdfreader.h"

#include "idblock.h"
#include "dg4block.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "channelobserver.h"


using namespace util::log;
using namespace util::string;
using namespace std::chrono_literals;

namespace mdf {

bool IsMdfFile(const std::string &filename) {
  auto *file = std::fopen(filename.c_str(), "rb");
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
  if (util::string::IEquals(oId.FileId(), "MDF", 3) ||
      util::string::IEquals(oId.FileId(), "UnFinMF", 7)) {
    return true;
  }

  return false;
}

/// Creates a channel sample observer. The sample observer creates internal memory for
/// all samples.\n
/// The function also attach the observer to the notifier (see observer pattern).
/// The destructor of the channel observer detach the observer.
/// \param data_group
/// \param group
/// \param channel
/// \return Smart pointer to a channel observer
ChannelObserverPtr CreateChannelObserver(const IDataGroup &data_group,
                                         const IChannelGroup &group,
                                         const IChannel &channel) {
  std::unique_ptr<IChannelObserver> observer;
  const uint32_t bytes = channel.DataBytes();

  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
      if (bytes <= 1) {
        observer = std::make_unique<detail::ChannelObserver<uint8_t>>(data_group, group, channel);
      } else if (bytes <= 2) {
        observer = std::make_unique<detail::ChannelObserver<uint16_t>>(data_group, group, channel);
      } else if (bytes <= 4) {
        observer = std::make_unique<detail::ChannelObserver<uint32_t>>(data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<uint64_t>>(data_group, group, channel);
      }
      break;

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
      if (bytes <= 1) {
        observer = std::make_unique<detail::ChannelObserver<int8_t>>(data_group, group, channel);
      } else if (bytes <= 2) {
        observer = std::make_unique<detail::ChannelObserver<int16_t>>(data_group, group, channel);
      } else if (bytes <= 4) {
        observer = std::make_unique<detail::ChannelObserver<int32_t>>(data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<int64_t>>(data_group, group, channel);
      }
      break;



    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
      if (bytes <= 4) {
        observer = std::make_unique<detail::ChannelObserver<float>>(data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<double>>(data_group, group, channel);
      }
      break;

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
      observer = std::make_unique<detail::ChannelObserver<std::string>>(data_group, group, channel);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      observer = std::make_unique<detail::ChannelObserver< std::vector<uint8_t> > >(data_group, group, channel);
      break;


    case ChannelDataType::CanOpenDate: // Convert to ms since 1970
    case ChannelDataType::CanOpenTime:
      observer = std::make_unique<detail::ChannelObserver<uint64_t>>(data_group, group, channel);
      break;

    default: break;
  }
  return std::move(observer);
}

ChannelObserverPtr CreateChannelObserver(const IDataGroup &dg_group, const std::string &channel_name) {
  std::unique_ptr<IChannelObserver> observer;

  const IChannelGroup* channel_group = nullptr;
  const IChannel* channel = nullptr;
  uint64_t nof_samples = 0;
  const auto cg_list = dg_group.ChannelGroups();
  for (const auto* cg_group : cg_list) {
    if (cg_group == nullptr) {
      continue;
    }
    const auto cn_list = cg_group->Channels();
    for (const auto* cn_item : cn_list) {
      if (cn_item == nullptr) {
        continue;
      }
      if (IEquals(channel_name,cn_item->Name())) {
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
                                          ChannelObserverList& dest) {
  auto cn_list = group.Channels();
  for (const auto* cn : cn_list) {
    if (cn != nullptr) {
      dest.emplace_back(std::move(CreateChannelObserver(data_group, group, *cn )));
    }
  }
}



MdfReader::MdfReader(const std::string &filename)
    : filename_(filename) {
  // Need to create MDF3 of MDF4 file
  bool bExist = false;
  try {
    std::filesystem::path p(filename_);
    if (std::filesystem::exists(p)) {
      bExist = true;
    }
  } catch (const std::exception &error) {
    LOG_ERROR() << "File I/O error. Filename: " << filename_
                << ", Error: " << error.what();
    return;
  }

  if (!bExist) {
    LOG_ERROR() << "The file doesn't exist. Filename: " << filename_;
    // No meaning to continue if the file doesn't exist
    return;
  }
  bool open = Open();
  if (!open || file_ == nullptr) {
    LOG_ERROR() << "The file couldn't be opened for reading (locked?). Filename: " << filename;
    // No meaning to continue if the file doesn't exist
    return;

  }
  std::unique_ptr<detail::IdBlock> id_block = std::make_unique<detail::IdBlock>();
  id_block->Read(file_);
  if (util::string::IEquals(id_block->FileId(), "MDF", 3) ||
      util::string::IEquals(id_block->FileId(), "UnFinMF", 7)) {
    if (id_block->Version() >= 400) {
      instance_ = std::make_unique<detail::Mdf4File>(std::move(id_block));
      instance_->Name(ShortName());
      instance_->FileName(filename_);
    } else {
      instance_ = std::make_unique<detail::Mdf3File>(std::move(id_block));
      instance_->Name(ShortName());
      instance_->FileName(filename_);
    }
    if (!instance_) {
      Close();
      LOG_ERROR() << "MDF version not supported. File: " << filename_;
    }
  } else {
    LOG_ERROR() << "This is not and MDF file. File: " << filename_;
    Close();
  }
}

MdfReader::~MdfReader() {
  Close();
}

std::string MdfReader::ShortName() const {
  try {
    std::filesystem::path file(filename_);
    return file.stem().string();
  } catch (const std::exception&) {
  }
  return {};
}

bool MdfReader::Open() {
  if (file_ != nullptr) {
    Close();
  }

  for (size_t ii = 0; ii < 6000; ++ii) {
    auto open = fopen_s(&file_, filename_.c_str(), "rb");
    switch (open) {
      case EEXIST:
      case EACCES:
        if (file_ != nullptr) {
          fclose(file_);
          file_ = nullptr;
        }
        std::this_thread::sleep_for(10ms);
        break;

      case ENOENT:
        if (file_ != nullptr) {
          fclose(file_);
          file_ = nullptr;
        }
        LOG_ERROR() << "File doesn't exist. File: " << filename_;
        return false;

      default:
        if (open != 0) {
          if (file_ != nullptr) {
            fclose(file_);
            file_ = nullptr;
          }
          LOG_ERROR() << "Failed to open the file. File: " << filename_
                      << ". Error: " << strerror(open) << " (" << open << ")";
          return false;
        }
        ii = 6000;
        break;
    }
  }
  if (file_ == nullptr) {
    LOG_ERROR() << "Failed to open the file due to lock timeout (5 s). File: " << filename_;
  }
  return file_ != nullptr;

}

void MdfReader::Close() {
  if (file_ != nullptr) {
    fclose(file_);
    file_ = nullptr;
  }
}

bool MdfReader::ReadHeader() {
  if (!instance_) {
    LOG_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
    // If file is not open then open and close the file in this call
  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    LOG_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  if (file_ == nullptr) {
    LOG_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadHeader(file_);
  } catch (const std::exception &error) {
    LOG_ERROR() << "Initialization failed. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadMeasurementInfo() {
  if (!instance_) {
    LOG_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    LOG_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadMeasurementInfo(file_);

  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to read the DG/CG blocks. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadEverythingButData() {
  if (!instance_) {
    LOG_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    LOG_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadEverythingButData(file_);

  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to read the file information blocks. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadAttachmentData(const IAttachment &attachment, const std::string &dest_file) {
  if (!instance_) {
    LOG_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    LOG_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    auto& at4 = dynamic_cast<const detail::At4Block&>(attachment);
    at4.ReadData(file_, dest_file);
  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to read the file information blocks. Error: " << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadData(const IDataGroup &data_group) {
  if (!instance_) {
    LOG_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = file_ == nullptr && Open();
  if (file_ == nullptr) {
    LOG_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    if (instance_->IsMdf4()) {
      const auto& dg4 = dynamic_cast<const detail::Dg4Block&>(data_group);
      dg4.ReadData(file_);
    } else {
      const auto& dg3 = dynamic_cast<const detail::Dg3Block&>(data_group);
      dg3.ReadData(file_);

    }
  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to read the file information blocks. Error: " << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

const IDataGroup *MdfReader::GetDataGroup(size_t order) const {
  const auto* file = GetFile();
  if (file != nullptr) {
    DataGroupList dg_list;
    file->DataGroups(dg_list);
    if (order >= 0 && order < dg_list.size()) {
      return dg_list[order];
    }
  }
  return nullptr;
}

}

