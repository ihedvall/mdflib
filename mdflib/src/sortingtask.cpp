/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "sortingtask.h"

#include <exception>
#include <filesystem>
#include <sstream>

#include "mdf/dgcomment.h"
#include "mdf/mdffactory.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"

#include "sortingconfigadapter.h"

using namespace std::filesystem;

namespace mdf {

void SortingTask::Run() {
  try {
    Result(false);
    CheckSourceFile();
    CheckDestinationFile();
    CreateDestinationTempFile();
    SortFile();
    CopyTempFile();
    DeleteTempFile();
    Error(false);
    Result(true);
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Failed to run the sorting task. Error: " << err.what()
        << ", File: " << SourceFile();
    MDF_ERROR() << oss.str();
    Error(true);
    SaveMessage(oss.str());
  }
}

void SortingTask::SortFile() {
  const std::string& source_path = SourceFile();
  const std::string& destination_path = DestinationFile();
  const std::string& temp_path = TempFile();
  try {
    if (source_path.empty()
       || destination_path.empty()
       || temp_path.empty()) {
      throw std::runtime_error("Invalid (empty) file names.");
    }
    const path source(source_path);
    const std::string source_name = source.stem().string();

    const path destination(destination_path);
    if (source == destination) {
      throw std::runtime_error("Source and destination files cannot be the same.");
    }

    reader_ = MdfFactory::CreateMdfReader(source_path);
    if (!reader_) {
      throw std::runtime_error("Failed to create the reader.");
    }
    const bool read = reader_->ReadEverythingButData();
    if (!read) {
      throw std::runtime_error(
        "Failed to read the source file. Source: " + source_name);
    }

    const MdfFile* source_file = reader_->GetFile();
    if (source_file == nullptr) {
      throw std::runtime_error(
        "Failed to get the MDF file. File: " + source_name);
    }

    const IHeader* source_header = source_file->Header();
    if (source_header == nullptr) {
      throw std::runtime_error(
        "Failed to get the header. File: " + source_name);
    }

    const path temp(temp_path);
    const std::string temp_name = temp.stem().string();
    writer_ = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
    if (!writer_) {
      throw std::runtime_error(
        "Failed to create the writer. File: " + temp_name);
    }
    writer_->StorageType(StorageType());
    writer_->CompressData(CompressData());

    writer_->Init(temp_path);
    MdfFile* dest_file = writer_->GetFile();
    if (dest_file == nullptr) {
      throw std::runtime_error(
        "Failed to get the destination file. File: " + temp_name);
    }
    IHeader* dest_header = dest_file->Header();
    if (dest_header == nullptr) {
      throw std::runtime_error(
        "Failed to get the destination header. File: " + temp_name);
    }

    SortingConfigAdapter config_adapter(*writer_, *reader_);
    config_adapter.CreateConfig(*dest_header);

    size_t dest_dg_count = 0;
    const auto source_dg_list = source_header->DataGroups();
    for (const IDataGroup* source_dg : source_dg_list) {
      if (source_dg == nullptr) {
        continue;
      }
      const auto source_cg_list = source_dg->ChannelGroups();
      for (const IChannelGroup* source_cg : source_cg_list) {
        if (source_cg == nullptr) {
          continue;
        }
        if ((source_cg->Flags() & CgFlag::VlsdChannel) != 0) {
          // The data should be stored in a SD block
          continue;
        }

        IDataGroup* dest_dg = dest_header->CreateDataGroup();
        if (dest_dg == nullptr) {
          throw std::runtime_error(
            "Failed to create the destination data group.");
        }
        ++dest_dg_count;
        DgComment comment;
        source_dg->GetDgComment(comment);
        if (comment.Comment().IsEmpty()) {
          std::ostringstream oss;
          oss << "Measurement " << dest_dg_count;
          comment.Comment(oss.str());
        }
        dest_dg->SetDgComment(comment);

        IChannelGroup* dest_cg = dest_dg->CreateChannelGroup();
        if (dest_cg == nullptr) {
          throw std::runtime_error(
            "Failed to create the destination channel group.");
        }
        dest_cg->CopyFrom(*source_cg);
        if (const ISourceInformation* source_si = source_cg->SourceInformation();
            source_si != nullptr) {
          ISourceInformation* dest_si = dest_cg->CreateSourceInformation();
          if (dest_si == nullptr) {
            throw std::runtime_error(
              "Failed to create the destination source information.");
          }
          dest_si->CopyFrom(*source_si);
        }
        const auto source_cn_list = source_cg->Channels();
        for (const IChannel* source_cn : source_cn_list) {
          if (source_cn == nullptr) {
            continue;
          }
          IChannel* dest_cn = dest_cg->CreateChannel();
          if (dest_cn == nullptr) {
            throw std::runtime_error(
              "Failed to create the destination channel.");
          }
          dest_cn->CopyFrom(*source_cn);

        }


      }
    }
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Failed to sort the file. Error: " << err.what();
    Error(true);
    throw std::runtime_error(oss.str());
  }

}

}  // namespace mdf