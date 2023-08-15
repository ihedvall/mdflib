/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/mdffile.h"

#include <filesystem>
#include <string>

#include "mdf/mdflogstream.h"

namespace mdf {

int MdfFile::MainVersion() const {
  const auto version = Version();
  int main_version = 0;
  try {
    const auto main = version.substr(0, 1);
    main_version = std::stoi(main);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failure to finding MDF main version. Error: " << err.what();
  }
  return main_version;
}

int MdfFile::MinorVersion() const {
  const auto version = Version();
  int minor_version = 0;

  try {
    const auto minor = version.substr(2);
    minor_version = std::stoi(minor);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failure to finding MDF minor version. Error: "
                << err.what();
  }
  return minor_version;
}

IAttachment* MdfFile::CreateAttachment() { return nullptr; }

void MdfFile::FileName(const std::string& filename) {
  filename_ = filename;
  try {
    auto temp = std::filesystem::u8path(filename);
    if (name_.empty()) {
      name_ = temp.stem().string();
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Invallid file name detected. Error: " << err.what()
                << ", File: " << filename;
  }
}

}  // namespace mdf
