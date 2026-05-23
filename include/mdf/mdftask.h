/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */


#pragma once

#include <memory>
#include <string>
#include <system_error>

#include "mdf/mdfenumerates.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
namespace mdf {
class MdfTask {
public:

  virtual ~MdfTask();

  void SourceFile(std::string source_file) {
    source_file_ = std::move(source_file);
  }
  [[nodiscard]] const std::string& SourceFile() const { return source_file_; }

  void DestinationFile(std::string dest_file) {
    destination_file_ = std::move(dest_file);
  }
  [[nodiscard]] const std::string& DestinationFile() const { return destination_file_; }

  void TempFile(std::string temp_file) {
    temp_file_ = std::move(temp_file);
  }
  [[nodiscard]] const std::string& TempFile() const { return temp_file_; }

  void StorageType(MdfStorageType storage_type) { storage_type_ = storage_type; }
  [[nodiscard]] MdfStorageType StorageType() const { return storage_type_; }

  void CompressData(bool compress) { compress_data_ = compress; }
  [[nodiscard]] bool CompressData() const { return compress_data_; }

  void OverwriteFile(bool overwrite) { overwrite_file_ = overwrite; }
  [[nodiscard]] bool OverwriteFile() const { return overwrite_file_; }

  void Version(MdfVersion version) { version_ = version; }
  [[nodiscard]] MdfVersion Version() const { return version_; }

  virtual void Run() = 0;

  void Result(bool result) const { result_ = result; }
  [[nodiscard]] bool Result() const { return result_; }

  void Error(bool error) const { error_ = error; }
  [[nodiscard]] bool Error() const { return error_; }

  void ErrorMessage(std::string message) const { error_message_ = std::move(message); }
  [[nodiscard]] const std::string& ErrorMessage() const { return error_message_; }

  [[nodiscard]] bool IsMdf4File() const { return is_mdf4_file_; }
  void SetMdf4File(bool is_mdf4_file) { is_mdf4_file_ = is_mdf4_file; }
protected:
  std::unique_ptr<MdfReader> reader_;
  std::unique_ptr<MdfWriter> writer_;

  MdfTask() = default;
  void CheckSourceFile() const;
  void CreateSourceTempFile();
  void CopyTempFile() const;
  void DeleteTempFile();
  [[nodiscard]] bool ValidateAllValues();
  void AddValidationResult();
  
private:
  std::string source_file_;
  std::string destination_file_;
  std::string temp_file_;

  mutable bool result_ = false;
  mutable bool error_ = false;
  mutable std::string error_message_;
  bool is_mdf4_file_ = true;

  MdfStorageType storage_type_ = MdfStorageType::FixedLengthStorage;
  bool compress_data_ = false;
  bool overwrite_file_ = false;
  MdfVersion version_ = MdfVersion::Mdf4_2;
};

}  // namespace mdf


