/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdftask.h"

#include <filesystem>
#include <sstream>

#include "mdf/fhcomment.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/ifilehistory.h"
#include "mdf/mdffactory.h"
#include "mdf/mdflogstream.h"

#include "sortingconfigadapter.h"

using namespace std::filesystem;

namespace mdf {

MdfTask::~MdfTask() {
  try {
    writer_.reset();
    reader_.reset();
    DeleteTempFile();
  } catch (const std::exception& err) {
    MDF_ERROR() << "Deleting temporary objects failed. Error: " << err.what();
  }

}

void MdfTask::SaveMessage(std::string message) const {
  if (message.empty()) {
    return;
  }
  message_list_.emplace_back(std::move(message));
}

void MdfTask::CheckSourceFile() const {
  try {
    const path source(SourceFile());
    if (source.empty()) {
      throw std::runtime_error("Source filename is empty");
    }
    if (!exists(source) || !is_regular_file(source)) {
      throw std::runtime_error(
        "Source file does not exist or is not a regular file" );
    }
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Check of source file failed."
    << " File: " << SourceFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
}

void MdfTask::CheckDestinationFile() const {
  try {
    const path dest(DestinationFile());
    if (dest.empty()) {
      // Destination file must be set
      throw std::runtime_error("Destination file is empty");
    }
    // Create dessnation directory if it doesn't exist
    const path parent = dest.parent_path();
    create_directories(parent);

    if (exists(dest) && !OverwriteFile()) {
      throw std::runtime_error(
        "Destination file already exists and overwrite is disabled");
    }

    const path source(SourceFile());
    if (source == dest) {
      throw std::runtime_error(
        "Source and destination files cannot be the same"
      );
    }
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Check of source file failed."
    << " File: " << SourceFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
}

void MdfTask::CheckSourceAndDestinationDiff() const {
  const path source_path(SourceFile());
  if (source_path.empty()) {
    throw std::runtime_error("The source path is empty.");
  }

  const path destination_path(DestinationFile());
  if (destination_path.empty()) {
    throw std::runtime_error("The destination path is empty.");
  }

  if (source_path == destination_path) {
    std::ostringstream temp;
    temp << "Source and destination path cannot be the same. Source: "
         << source_path << ", Destination: " << destination_path;
    throw std::runtime_error(temp.str());
  }
}

void MdfTask::CreateSourceTempFile() {
  try {
    const path source(SourceFile());
    path temp( temp_directory_path());
    temp /= source.filename();

    if (temp == source) {
      throw std::runtime_error(
        "Cannot create temporary file with same name as source file"
      );
    }

    remove(temp);
    copy(source, temp);
    TempFile(temp.string());
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Creating a temporary file failed."
    << " File: " << SourceFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
}

void MdfTask::CreateDestinationTempFile() {
  try {
    const path dest(DestinationFile());
    path temp( temp_directory_path());
    temp /= dest.filename();

    if (temp == dest) {
      throw std::invalid_argument(
"Cannot create temporary file with same path as the destination file. Path: "
    + DestinationFile() );
    }

    remove(temp);

    TempFile(temp.string());
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Creating a temporary file failed."
    << " File: " << DestinationFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
}

void MdfTask::CreateWriter() {
  const path temp_path(TempFile());
  if (temp_path.empty()) {
    throw std::invalid_argument("Temporary file path is empty. File: "
      + TempFile());
  }

  writer_ = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
  if (!writer_) {
    throw std::logic_error("Failed to create the writer. File: "
      + TempFile());
  }
  writer_->StorageType(StorageType());
  writer_->CompressData(CompressData());

  const bool init = writer_->Init(temp_path.string());
  if (!init) {
    throw std::runtime_error("Failed to initialize the writer. File: "
      + TempFile());
  }

}

void MdfTask::CreateObserverForTopLevelChannels(
    const IDataGroup& data_group, const IChannelGroup& channel_group,
    ChannelObserverList& observer_list) {
  const auto channel_list = channel_group.TopLevelChannels();
  for (const IChannel* channel : channel_list) {
    if (channel == nullptr) {
      continue;
    }
    auto observer = CreateChannelObserver(data_group,channel_group, *channel);
    observer_list.emplace_back(std::move(observer));
  }
}

void MdfTask::CopyTempFile() const {
  try {
    const path temp(TempFile());
    path dest;
    if (!DestinationFile().empty()) {
      dest = DestinationFile();
    } else {
      dest = SourceFile();
    }
    if (exists(dest)) {
      if (OverwriteFile()) {
        copy_file(temp, dest, std::filesystem::copy_options::overwrite_existing);
      }
    } else {
      copy_file(temp, dest);
    }
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Copying a temporary file failed."
    << " File: " << TempFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
}

void MdfTask::CopyMainConfig() const {
  if (!reader_ || !writer_) {
    throw std::invalid_argument("Reader or writer is not initialized.");
  }
  const MdfFile* source_file = reader_->GetFile();
  if (source_file == nullptr) {
    throw std::runtime_error(
      "Failed to get the source MDF file. File: " + SourceFile());
  }

  const IHeader* source_header = source_file->Header();
  if (source_header == nullptr) {
    throw std::runtime_error(
      "Failed to get the source MDF header. File: " + SourceFile());
  }

  MdfFile* dest_file = writer_->GetFile();
  if (dest_file == nullptr) {
    throw std::runtime_error(
      "Failed to get the temporary MDF file. File: " + TempFile());
  }
  IHeader* dest_header = dest_file->Header();
  if (dest_header == nullptr) {
    throw std::runtime_error(
      "Failed to get the temporary MDF header. File: " + TempFile());
  }

  SortingConfigAdapter config_adapter(*writer_, *reader_);
  config_adapter.CreateConfig(*dest_header);

}

IChannelGroup* MdfTask::CopyChannelConfig(const IChannelGroup& source_cg,
                                IDataGroup& dest_dg) {
  IChannelGroup* dest_cg = dest_dg.CreateChannelGroup();
  if (dest_cg == nullptr) {
    throw std::logic_error(
      "Failed to create the destination channel group.");
  }

  dest_cg->CopyFrom(source_cg);
  if (const ISourceInformation* source_si = source_cg.SourceInformation();
      source_si != nullptr) {
    ISourceInformation* dest_si = dest_cg->CreateSourceInformation();
    if (dest_si == nullptr) {
      throw std::logic_error(
        "Failed to create the destination source information.");
    }
    dest_si->CopyFrom(*source_si);
  }
  const auto source_cn_list = source_cg.TopLevelChannels();
  for (const IChannel* source_cn : source_cn_list) {
    if (source_cn == nullptr) {
      continue;
    }
    IChannel* dest_cn = dest_cg->CreateChannel();
    if (dest_cn == nullptr) {
      throw std::logic_error("Failed to create the destination channel.");
    }
    dest_cn->CopyConfigFrom(*source_cn);
  }
  return dest_cg;
}

void MdfTask::DeleteTempFile() {
  if (TempFile().empty()) {
    return;
  }
  try {
    const path temp(TempFile());
    remove(temp);
    TempFile("");
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Deleting a temporary file failed."
    << " File: " << TempFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
}

bool MdfTask::ValidateAllValues() {
  bool all_valid = true;
  try {
    reader_ = MdfFactory::CreateMdfReader(TempFile());
    if (!reader_) {
      throw std::runtime_error("Failed to create the MDF reader.");
    }
    if (const bool file_ok = reader_->IsOk(); !file_ok ) {
      throw std::runtime_error("File is not OK.");
    }
    if (const bool read_config = reader_->ReadEverythingButData(); !read_config) {
      throw std::runtime_error("Failed to read the MDF configuration.");
    }
    const MdfFile* mdf_file = reader_->GetFile();
    if (mdf_file == nullptr) {
      throw std::runtime_error("Failed to get the MDF file.");
    }

    SetMdf4File(mdf_file->IsMdf4()); // Used  later when updating the file.

    DataGroupList dg_list;
    mdf_file->DataGroups(dg_list);
    if (dg_list.empty()) {
      throw std::runtime_error("No data groups in the file.");
    }
    size_t dg_order = 0;
    for (IDataGroup* data_group : dg_list) {
      ++dg_order;
      if (data_group == nullptr) {
        continue;
      }

      ChannelObserverList channel_list;
      CreateChannelObserverForDataGroup(*data_group, channel_list);
      if (const bool read_data = reader_->ReadData(*data_group); !read_data) {
        throw std::runtime_error("Failed to read the sample data.");
      }
      for (const auto& channel_observer : channel_list) {
        if (!channel_observer) {
          continue;
        }
        const auto& valid_list = channel_observer->GetValidList();
        const bool valid_values = std::all_of(valid_list.cbegin(),
          valid_list.cend(),
          [](const auto& valid) {
          return valid;
        });
        if (!valid_values) {
          all_valid = false;
          std::string dg_name = data_group->Description();
          if (dg_name.empty()) {
            dg_name = "Measurement " + std::to_string(dg_order);
          }
          std::ostringstream err_msg;
          err_msg << "Invalid values found. Data Group: " << dg_name
            << " Channel Group: " << channel_observer->ChannelGroupName()
            << ", Channel: " << channel_observer->Name();
          SaveMessage(err_msg.str());
        }
      }
      data_group->ClearData();
    }
    reader_.reset();
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Validating all values function failed."
    << " File: " << TempFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());
  }
  reader_.reset();

  return all_valid;
}

void MdfTask::AddValidationResult() {
  try {
    writer_ = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    if (!writer_) {
      throw std::runtime_error("Failed to create the MDF writer.");
    }
    if (const bool init = writer_->Init(TempFile()); !init) {
      throw std::runtime_error("Failed to initialize the MDF writer.");
    }
    if (const bool new_file = writer_->IsFileNew(); new_file) {
      throw std::runtime_error("The MDF file is a new file. Must be an existing file");
    }
    IHeader* header = writer_->Header();
    if (header == nullptr) {
      throw std::runtime_error("Failed to get the MDF header.");
    }

    IFileHistory* file_history = header->CreateFileHistory();
    if (file_history == nullptr) {
      throw std::runtime_error("Failed to create the MDF file history.");
    }
    file_history->Time(MdfHelper::NowNs());

    FhComment fh_comment("Valid");
    file_history->SetFhComment(fh_comment);

    if (const bool finalize = writer_->FinalizeMeasurement(); !finalize) {
      throw std::runtime_error("Failed to finalize the MDF measurement.");
    }

    writer_.reset();
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Adding validation result failed."
    << " File: " << TempFile()
    << ", Error: " << err.what();
    MDF_ERROR() << oss.str();
    throw std::runtime_error(oss.str());

  }
}

void MdfTask::ReadConfig() {
  path source(SourceFile());
  if (!exists(source)) {
    throw std::runtime_error("Source file does not exist. File: "
      + SourceFile());
  }
  reader_ = MdfFactory::CreateMdfReader(SourceFile());
  if (!reader_) {
    throw std::runtime_error("Failed to create the reader object.");
  }
  if (!reader_->IsOk()) {
    throw std::runtime_error("The source file isn't OK. File: "
      + SourceFile() );
  }
  const bool read = reader_->ReadEverythingButData();
  if (!read) {
    throw std::runtime_error("Failed to read the configuration . File: "
      + SourceFile() );
  }
}

}  // namespace mdf