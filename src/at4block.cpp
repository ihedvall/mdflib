/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <filesystem>
#include <sstream>
#include <cerrno>
#include "at4block.h"
#include "util/zlibutil.h"
#include "util/cryptoutil.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexFilename = 1;
constexpr size_t kIndexType = 2;
constexpr size_t kIndexMd = 3;

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & mdf::detail::At4Flags::kEmbeddedData) {
    s << "Embedded";
  }
  if (flag & mdf::detail::At4Flags::kCompressedData) {
    s << (s.str().empty() ? "Compressed" : ",Compressed");
  }
  if (flag & mdf::detail::At4Flags::kUsingMd5) {
    s << (s.str().empty() ? "MD5" : ",MD5");
  }
  return s.str();
}

bool CopyBytes(std::FILE* source, std::FILE* dest, uint64_t nof_bytes) {
  uint8_t temp = 0;
  for (uint64_t ii = 0; ii < nof_bytes; ++ii) {
    if (fread(&temp, 1, 1, source) != 1) {
      return false;
    }
    if (fwrite(&temp, 1, 1, dest) != 1) {
      return false;
    }
  }
  return true;
}

} // namespace

namespace mdf::detail {

void At4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next AT", ToHexString(Link(kIndexNext)), "Link to next attach", BlockItemType::LinkItem );
  dest.emplace_back("File Name TX", ToHexString(Link(kIndexFilename)), "Link to text block with file name",BlockItemType::LinkItem );
  dest.emplace_back("Mime Type TX", ToHexString(Link(kIndexType)), "Link to file type (MIME)", BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  std::string name;
  if (Link(kIndexFilename) > 0 ) {
    try {
      std::filesystem::path p(filename_);
      name = p.filename().string();
    } catch (const std::exception&) {
      name = "<invalid>";
    }
    dest.emplace_back("File Name", name, name == filename_ ? "" : filename_);
  }
  if (Link(kIndexType) > 0 ) {
    dest.emplace_back("File Type", file_type_);
  }
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Creator Index", std::to_string(creator_index_));
  if (flags_ & At4Flags::kUsingMd5) {
    dest.emplace_back("MD5 Checksum", ToMd5String(md5_));
  }
  dest.emplace_back("Original Size [byte]", std::to_string(original_size_));
  dest.emplace_back("Embedded Size [byte]", std::to_string(nof_bytes_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t At4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, creator_index_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 4);
  bytes += ReadByte(file, md5_, 16);
  bytes += ReadNumber(file, original_size_);
  bytes += ReadNumber(file, nof_bytes_);
  // Do not read in the data BLOB at this point but store the file position for that
  // data, so it is fast to get the data later
  data_position_ = GetFilePosition(file);

  filename_ = ReadTx4(file, kIndexFilename);
  file_type_ = ReadTx4(file, kIndexType);
  ReadMdComment(file, kIndexMd);

  return bytes;
}

void At4Block::ReadData(std::FILE *file, const std::string &dest_file) const {
  if (file == nullptr || data_position_ <= 0) {
    throw std::invalid_argument("File is not opened or data position not read");
  }
  SetFilePosition(file,data_position_);
  if (IsEmbedded()) {
    auto* dest = fopen(dest_file.c_str(), "wb");
    if (dest == nullptr) {
      throw std::ios_base::failure("Failed to open the destination file");
    }
    const bool error = IsCompressed() ? !util::zlib::Inflate(file,dest,nof_bytes_)
        : !CopyBytes(file, dest, nof_bytes_);
     fclose(dest);
    if (error) {
      throw std::ios_base::failure("Failed to copy correct number of bytes");
    }
  } else {
    // Need to copy the source file
    std::filesystem::path s(filename_);
    std::filesystem::path d(dest_file);
    if (s != d) {
      std::filesystem::copy_file(s,d, std::filesystem::copy_options::overwrite_existing);
    }
  }
  if (flags_ & At4Flags::kUsingMd5) {
    std::vector<uint8_t> md5(16,0xFF);
    util::crypto::CreateMd5FileChecksum(dest_file, md5);
    if (md5 != md5_) {
      throw std::runtime_error("Mismatching MD5 checksums");
    }
  }
}

}