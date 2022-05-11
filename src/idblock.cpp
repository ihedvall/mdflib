/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <ios>

#ifdef WIN32
#include <windows.h>
#endif

#include "util/stringutil.h"
#include "idblock.h"

using namespace std;
namespace mdf::detail {
IdBlock::IdBlock() {
  block_type_ = "ID";
  block_length_ = 64;
  block_size_ = 64;
  file_position_ = -1; // Set to -1 which indicate that is neither read nor written to disc. 0 is
  // its file position when written or read.
}

void IdBlock::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("File ID", FileId());
  dest.emplace_back("Version", VersionString());
  dest.emplace_back("Program", ProgramId());
  if (version_ < 400) {
    dest.emplace_back("Byte Order",
                      byte_order_ == 0 ? "Little Endian" : "Big Endian",
                      byte_order_ == 0 ? "Intel byte order" : "Motorola byte order");
    dest.emplace_back("Version Number", std::to_string(version_));
#ifdef WIN32
    CPINFOEXA cp_info {};
    GetCPInfoExA(code_page_number_, CP_ACP, &cp_info);
    dest.emplace_back("Code Page", std::to_string(code_page_number_),cp_info.CodePageName);
#else
    dest.emplace_back("Code Page", std::to_string(code_page_number_));
#endif
    dest.emplace_back("Version Number", std::to_string(version_) );
    dest.emplace_back("Update CG count", (standard_flags_ & 0x1) != 0 ? "True" : "False");
    dest.emplace_back("Update SR Count", (standard_flags_ & 0x1) != 0 ? "True" : "False");
    dest.emplace_back("Custom Flags", std::to_string(custom_flags_));
  }

}

size_t IdBlock::Read(std::FILE *file) {
  block_type_ = "ID";
  block_length_ = 64;

  file_position_ = GetFilePosition(file);
  size_t bytes = ReadStr(file, file_identifier_, 8);
  bytes += ReadStr(file, format_identifier_, 8);
  bytes += ReadStr(file, program_identifier_, 8);
  bytes += ReadNumber(file, byte_order_); // Note defined in IBlock class
  bytes += ReadNumber(file, floating_point_format_);
  bytes += ReadNumber(file, version_);
  bytes += ReadNumber(file, code_page_number_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 28);
  bytes += ReadNumber(file, standard_flags_);
  bytes += ReadNumber(file, custom_flags_);
  return bytes;
}
size_t IdBlock::Write(std::FILE *file) {
  // Check if it has been written. Update is not supported in ID
  if (FilePosition() == 0) {
    return 64;
  }

//  Do not call IBlock::Write(file) as the other block writes do
  if (file == nullptr) {
    throw std::runtime_error("File pointer is null");
  }
  SetFirstFilePosition(file);
  file_position_ = GetFilePosition(file);
  size_t bytes = WriteStr(file, file_identifier_, 8);
  bytes += WriteStr(file, format_identifier_, 8);
  bytes += WriteStr(file, program_identifier_, 8);
  bytes += WriteNumber(file, byte_order_);
  bytes += WriteNumber(file, floating_point_format_);
  bytes += WriteNumber(file, version_);
  bytes += WriteNumber(file, code_page_number_);
  std::vector<uint8_t> reserved(28,0);
  bytes += WriteByte(file, reserved);
  bytes += WriteNumber(file, standard_flags_);
  bytes += WriteNumber(file, custom_flags_);
  if (bytes != 64) {
    throw std::runtime_error("ID block not 64 bytes");
  }
  return bytes;
}

std::string IdBlock::FileId() const {
  std::string temp = file_identifier_;
  util::string::Trim(temp);
  return temp;
}

std::string IdBlock::VersionString() const {
  std::string temp = format_identifier_;
  util::string::Trim(temp);
  return temp;
}

std::string IdBlock::ProgramId() const {
  std::string temp = program_identifier_;
  util::string::Trim(temp);
  return temp;
}

void IdBlock::SetDefaultMdf3Values() {
  file_identifier_ = "MDF     ";
  format_identifier_= "3.30";
  program_identifier_ = "MdfWrite";
  byte_order_ = 0; // Little endian
  floating_point_format_ = 0;
  version_ = 330;
  code_page_number_ = 65001; // UTF-8
  standard_flags_ = 0;
  custom_flags_ = 0;
}



}


