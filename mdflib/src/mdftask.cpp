/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdftask.h"

#include <filesystem>
#include <sstream>

#include "mdf/fhcomment.h"
#include "mdf/idatagroup.h"
#include "mdf/ifilehistory.h"
#include "mdf/mdffactory.h"
#include "mdf/mdflogstream.h"

using namespace std::filesystem;

namespace mdf {

MdfTask::~MdfTask() {
  try {
    reader_.reset();
    writer_.reset();
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
      throw std::runtime_error(
        "Cannot create temporary file with same name as the destination file"
      );
    }

    remove(temp);
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

}  // namespace mdf