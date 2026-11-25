/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdfconverter.h"
#include <chrono>
#include <sstream>
#include "mdf/mdflogstream.h"
#include "mdfblock.h"
#include "dg4block.h"
#include "dt4block.h"
#include "cn4block.h"
#include "hl4block.h"
#include "dl4block.h"
#include "dz4block.h"
#include "samplequeue.h"

using namespace std::chrono_literals;

namespace mdf::detail {
namespace {
class ConfigurationBuffer final : public std::stringbuf {
 public:
  ConfigurationBuffer() : std::stringbuf(std::ios_base::out | std::ios_base::binary) {}

 protected:
  // GetFilePosition() asks for in|out, which std::stringbuf refuses for cur.
  pos_type seekoff(off_type offset, std::ios_base::seekdir direction,
                   std::ios_base::openmode) override {
    return std::stringbuf::seekoff(offset, direction, std::ios_base::out);
  }
  pos_type seekpos(pos_type position, std::ios_base::openmode) override {
    return std::stringbuf::seekpos(position, std::ios_base::out);
  }
};
}

MdfConverter::MdfConverter() {
  type_of_writer_ = MdfWriterType::MdfConverter;
}

MdfConverter::~MdfConverter() {
  write_cache_.Exit();
  if (IsOpen()) {
    Close();
  }
}

bool MdfConverter::InitMeasurement() {
  write_cache_.Exit();

  if (IsOpen()) {
    Close();
  }

  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  // Set up internal sample buffers so the last channel values can be stored
  const bool prep = PrepareForWriting();
  if (!prep) {
    MDF_ERROR() << "Failed to prepare the file for writing. File: "
                << filename_.c_str();
    return false;
  }
  // 1: Save ID, HD, DG, AT, CG and CN blocks to the file.

  Open( write_state_ == WriteState::Create ?
    std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
    :  std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: "
      << filename_.c_str();
    return false;
  }

  // Save the configuration to disc
  bool write = false;
  AttachmentList attachments;
  mdf_file_->Attachments(attachments);
  if (write_state_ == WriteState::Create && !filename_.empty() && attachments.empty()) {
    try {
      ConfigurationBuffer configuration;
      if (mdf_file_->Write(configuration)) {
        const auto bytes = configuration.str();
        const auto size = static_cast<std::streamsize>(bytes.size());
        write = file_->sputn(bytes.data(), size) == size && file_->pubsync() == 0;
        if (!write) {
          MDF_ERROR() << "Failed to write the configuration. File: "
                      << filename_.c_str();
        }
      }
    } catch (const std::exception& err) {
      MDF_ERROR() << "Failed to write the configuration. Error: " << err.what()
                  << ", File: " << filename_.c_str();
    }
  } else {
    write = mdf_file_->Write(*file_);
  }
  //SetDataPosition();  // Set up data position to end of file

  // Keep the file open to save conversion time

  start_time_ = 0;  // Zero indicate not started
  stop_time_ = 0;   // Zero indicate not stopped
  // Start the working thread that handles the samples
  write_state_ = WriteState::Init;  // Waits for new samples
  write_cache_.Init();
  return write;
}

bool MdfConverter::FinalizeMeasurement() {
  write_cache_.Exit();

  // Save outstanding non-written blocks and any block updates as
  // sample counters which changes during DG/DT updates
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: "
      << filename_.c_str();
    return false;
  }
  const bool write = mdf_file_ && mdf_file_->Write(*file_);
  const bool signal_data = WriteSignalData(*file_);
  Close();

  write_state_ = WriteState::Finalize;
  return write && signal_data;
}



}  // namespace mdf