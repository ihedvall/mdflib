/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <filesystem>
#include <sstream>
#include <cerrno>
#include "at4block.h"
#include "mdf/mdfhelper.h"
#include "mdf/mdflogstream.h"
#include "mdf/zlibutil.h"
#include "mdf/cryptoutil.h"
#include "platform.h"

using namespace std::filesystem;

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexFilename = 1;
constexpr size_t kIndexType = 2;
constexpr size_t kIndexMd = 3;

using namespace mdf;

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

std::string ConvertMd5Buffer(const std::vector<uint8_t>& buffer) {
  std::ostringstream temp;
  for (auto byte : buffer ) {
    temp << std::uppercase <<  std::setfill('0')
         << std::setw(2) << std::hex << static_cast<uint16_t>(byte);
  }
  return temp.str();
}

bool FileToBuffer(const std::string& filename, mdf::ByteArray& dest) {
  try {
    path fullname(filename);
    const auto size = file_size(fullname);
    if (size > 0) {
      dest.resize(size, 0);
      std::FILE* file = nullptr;
      Platform::fileopen(&file, filename.c_str(), "rb");
      if (file != nullptr) {
        const auto nof_bytes = fread(dest.data(), 1, size, file);
        if (nof_bytes < size) {
          dest.resize(nof_bytes);
        }
      } else {
        MDF_ERROR() << "Failed to open file. File: " << filename;
        dest.clear();
        return false;
      }
    } else {
      dest.clear();
    }

  } catch (const std::exception& err) {
    MDF_ERROR() << "File error when reading file to byte array. Error: " << err.what() << ", File: " << filename;
    return false;
  }
  return true;
}

 } // namespace

namespace mdf::detail {
At4Block::At4Block() {
  block_type_ = "##AT";
}

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

size_t At4Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  ByteArray data_buffer;
  try {
   path filename(filename_);
   if (!std::filesystem::exists(filename)) {
     MDF_ERROR() << "Attachment File doesn't exist. File: " << filename_;
     return 0;
   }

   const auto md5 = CreateMd5FileChecksum(filename_, md5_);
   if (md5) {
     flags_ |= At4Flags::kUsingMd5;
   }

   original_size_ = file_size(filename);
   if (IsEmbedded() && IsCompressed()) {
     const bool compress = Deflate(filename_, data_buffer);
     if (!compress) {
       MDF_ERROR() << "Compress failure. File: " << filename;
       return 0;
     }
   } else if (IsEmbedded()) {
     const auto buffer = FileToBuffer(filename_, data_buffer);
     if (!buffer) {
       MDF_ERROR() << "File to buffer failure. File: " << filename;
       return 0;
     }
   }

  } catch (const std::exception& err) {
    MDF_ERROR() << "Attachment File error. Error: " << err.what() << ", File: " << filename_;
    return 0;
  }

  nof_bytes_ = data_buffer.size();

  block_type_ = "##AT";
  block_length_ = 24 + (4*8) + 2 + 2 + 4 + 16 + 8 + 8 + nof_bytes_;
  link_list_.resize(4,0);

  WriteTx4(file, kIndexFilename, filename_);
  WriteTx4(file, kIndexType, file_type_);
  WriteMdComment(file, kIndexMd);

  auto bytes = IBlock::Write(file);
  bytes += WriteNumber(file, flags_);
  bytes += WriteNumber(file, creator_index_);
  bytes += WriteBytes(file, 4);
  if (md5_.size() == 16) {
    bytes += WriteByte(file, md5_);
  } else {
    bytes += WriteBytes(file, 16);
  }
  bytes += WriteNumber(file, original_size_);
  bytes += WriteNumber(file, nof_bytes_);
  data_position_ = FilePosition();
  if (nof_bytes_ > 0) {
    bytes += WriteByte(file, data_buffer);
  }
  UpdateBlockSize(file,bytes);

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
    const bool error = IsCompressed() ? !Inflate(file,dest,nof_bytes_)
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
    CreateMd5FileChecksum(dest_file, md5);
    if (md5 != md5_) {
      throw std::runtime_error("Mismatching MD5 checksums");
    }
  }
}

void At4Block::IsEmbedded(bool embed) {
  if (embed) {
    flags_ |= At4Flags::kEmbeddedData;
  } else {
    flags_ &= ~At4Flags::kEmbeddedData;
  }
}
void At4Block::FileName(const std::string &filename) {
  filename_ = filename;
}
const std::string &At4Block::FileName() const {
  return filename_;
}
void At4Block::FileType(const std::string &file_type) {
  file_type_ = file_type;
}
const std::string &At4Block::FileType() const {
  return file_type_;
}
bool At4Block::IsEmbedded() const {
  return flags_ & At4Flags::kEmbeddedData;
}
void At4Block::IsCompressed(bool compress) {
  if (compress) {
    flags_ |= At4Flags::kCompressedData;
  } else {
    flags_ &= ~At4Flags::kCompressedData;
  }
}
bool At4Block::IsCompressed() const {
  return flags_ & At4Flags::kCompressedData;
}

std::optional<std::string> At4Block::Md5() const {
  if ((flags_ & At4Flags::kUsingMd5) == 0) {
    return {};
  }
  return ConvertMd5Buffer(md5_);
}

void At4Block::CreatorIndex(uint16_t creator) {
  creator_index_ = creator;
}
uint16_t At4Block::CreatorIndex() const {
  return creator_index_;
}

int64_t At4Block::Index() const {
  return FilePosition();
}


}