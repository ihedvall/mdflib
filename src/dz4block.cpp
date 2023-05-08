/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dz4block.h"

#include <mdf/zlibutil.h>

#include <cstdio>
#include <string>

namespace {

std::string MakeZipTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "ZLIB Deflate";
    case 1:
      return "Transposition + ZLIB Deflate";
    default:
      break;
  }
  return "Unknown";
}
}  // namespace

namespace mdf::detail {

void Dz4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Original Block Type", orig_block_type_);
  dest.emplace_back("Zip Type", MakeZipTypeString(type_));
  dest.emplace_back("Zip Parameter", std::to_string(parameter_));
  dest.emplace_back("Original Size  [byte]", std::to_string(orig_data_length_));
  dest.emplace_back("Data Size  [byte]", std::to_string(data_length_));
}

size_t Dz4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadStr(file, orig_block_type_, 2);
  bytes += ReadNumber(file, type_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 1);
  bytes += ReadNumber(file, parameter_);
  bytes += ReadNumber(file, orig_data_length_);
  bytes += ReadNumber(file, data_length_);
  data_position_ = GetFilePosition(file);
  return bytes;
}

size_t Dz4Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0;
  if (update) {
    // The DZ block properties cannot be changed after it has been written
    return block_length_;
  }

  block_type_ = "##DZ";
  link_list_.clear();
  block_length_ = 24 + 2 + 1 + 1 + 4 + 8 + 8 + data_length_;

  auto bytes = MdfBlock::Write(file);
  bytes += WriteStr(file, orig_block_type_, 2);
  bytes += WriteNumber(file, type_);
  bytes += WriteBytes(file, 1);
  bytes += WriteNumber(file, parameter_);
  bytes += WriteNumber(file, orig_data_length_);
  bytes += WriteNumber(file, data_length_);
  bytes += WriteByte(file,data_);
  UpdateBlockSize(file, bytes);

  return bytes;
}

size_t Dz4Block::CopyDataToFile(std::FILE *from_file,
                                std::FILE *to_file) const {
  if (data_position_ == 0 || orig_data_length_ == 0 || data_length_ == 0) {
    return 0;
  }
  SetFilePosition(from_file, data_position_);

  size_t count = 0;
  switch (static_cast<Dz4ZipType>(type_)) {
    case Dz4ZipType::Deflate: {
      const bool inflate = Inflate(from_file, to_file, data_length_);
      count = inflate ? orig_data_length_ : 0;
      break;
    }

    case Dz4ZipType::TransposeAndDeflate: {
      ByteArray temp(data_length_, 0);
      fread(temp.data(), 1, temp.size(), from_file);
      ByteArray out(orig_data_length_, 0);
      const bool inflate = Inflate(temp, out);
      InvTranspose(out, parameter_);
      fwrite(out.data(), 1, out.size(), to_file);
      count = inflate ? orig_data_length_ : 0;
      break;
    }

    default:
      break;
  }
  return count;
}

size_t Dz4Block::CopyDataToBuffer(std::FILE *from_file,
                                  std::vector<uint8_t> &buffer,
                                  size_t &buffer_index) const {
  if (data_position_ == 0 || orig_data_length_ == 0 || data_length_ == 0) {
    return 0;
  }

  SetFilePosition(from_file, data_position_);

  size_t count = 0;
  switch (static_cast<Dz4ZipType>(type_)) {
    case Dz4ZipType::Deflate: {
      ByteArray temp(data_length_, 0);
      fread(temp.data(), 1, temp.size(), from_file);

      ByteArray out(orig_data_length_, 0);
      Inflate(temp, out);
      count = orig_data_length_;
      memcpy(buffer.data() + buffer_index, out.data(), count);
      buffer_index += count;
      break;
    }

    case Dz4ZipType::TransposeAndDeflate: {
      ByteArray temp(data_length_, 0);
      fread(temp.data(), 1, temp.size(), from_file);

      ByteArray out(orig_data_length_, 0);
      Inflate(temp, out);
      InvTranspose(out, parameter_);

      count = orig_data_length_;
      memcpy(buffer.data() + buffer_index, out.data(), count);
      buffer_index += count;
      break;
    }

    default:
      break;
  }
  return count;
}

bool Dz4Block::Data(const std::vector<uint8_t> &uncompressed_data) {
  bool compress = true;
  if (Type() == Dz4ZipType::TransposeAndDeflate) {
    if (Parameter() == 0) {
      Type(Dz4ZipType::Deflate);
    }
  } else {
    Type(Dz4ZipType::Deflate);
    Parameter(0);
  }
  if (uncompressed_data.empty()) {
    orig_data_length_ = 0;
    data_length_ = 0;
    data_.clear();
    return compress;
  }

  switch (Type()) {
    case Dz4ZipType::TransposeAndDeflate: {
      ByteArray temp = uncompressed_data;
      Transpose(temp, static_cast<size_t>(parameter_));

      data_.clear();
      data_.reserve(uncompressed_data.size());

      compress = Deflate(temp,data_);
      break;
    }

    case Dz4ZipType::Deflate:
    default:
      data_.clear();
      data_.reserve(uncompressed_data.size());
      compress = Deflate(uncompressed_data,data_);
      break;
  }
  orig_data_length_ = static_cast<uint64_t>(uncompressed_data.size()) ;
  data_length_ = static_cast<uint64_t>(data_.size());

  return compress;
}

}  // namespace mdf::detail