/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <sstream>
#include <ios>
#include <boost/algorithm/string.hpp>

#include "iblock.h"
#include "md4block.h"
#include "tx3block.h"

namespace mdf::detail {

std::fpos_t GetFilePosition(std::FILE *file) {
  std::fpos_t curr = 0;
  auto get = std::fgetpos(file, &curr);
  if (get != 0) {
    throw std::ios_base::failure("Failed to get a file position");
  }
  return curr;
}

void SetFilePosition(std::FILE *file, std::fpos_t position) {
  std::fpos_t curr = 0;
  auto get = std::fgetpos(file, &curr);
  if (get != 0) {
    throw std::ios_base::failure("Failed to get a file position");
  }

  // Fast check if it already is in position

  if (curr == position) {
    return;
  }
  fpos_t temp = position;
  auto set = std::fsetpos(file, &temp);
  if (set != 0) {
    throw std::ios_base::failure("Failed to set a file position");
  }
  if (temp != position) {
    throw std::ios_base::failure("Failed to set a file position");
  }
}

void SetFirstFilePosition(std::FILE *file) {
#if (MSVC)
  _fseeki64(&file, 0, SEEK_SET);
#else
  fseeko64(file, 0, SEEK_SET);
#endif
}

size_t StepFilePosition(std::FILE *file, size_t steps) {
#if (MSVC)
  _fseeki64(&file, static_cast<int64_t>(steps) , SEEK_CUR);
#else
  fseeko64(file, static_cast<int64_t>(steps), SEEK_CUR);
#endif
  return steps;
}

std::string ReadBlockType(std::FILE *file) {
  std::string type3;
  ReadStr(file,type3,2);
  if (type3 == "##") {
    std::string type4;
    ReadStr(file, type4,2);
    return type4;
  }
  return type3;
}

std::size_t ReadByte(std::FILE *file, std::vector<uint8_t> &dest, const size_t size) {
  if (size == 0) {
    dest.clear();
    return 0;
  }
  dest.resize(size, 0);
  std::fread(dest.data(), 1, size, file);
  return size;
}
std::size_t WriteByte(std::FILE *file, const std::vector<uint8_t> &source) {
  return std::fwrite(source.data(), 1, source.size(), file);
}

size_t ReadStr(std::FILE *file, std::string &dest, const size_t size) {
  std::ostringstream temp;
  char in = '\0';
  for (size_t ii = 0; ii < size; ++ii) {
    auto byte = std::fread(&in, 1, 1, file);
    if (byte != 1) {
      std::ostringstream error;
      error << "Failed to read an MDF string. "
               "In byte " << ii << " of " << size << ".";
      throw std::ios_base::failure(error.str());
    }
    if (in != '\0') {
      temp << in;
    }
  }
  dest = temp.str();
  boost::trim(dest);
  return size;
}

std::size_t WriteStr(std::FILE *file, const std::string &source, size_t size) {
  for (size_t ii = 0; ii < size; ++ii) {
    const char in = ii < source.size() ? source[ii] : '\0';
    const auto byte = std::fwrite(&in, 1, 1, file);
    if (byte != 1) {
      std::ostringstream error;
      error << "Failed to write an MDF string. "
               "In byte " << ii << " of " << size << ".";
      throw std::runtime_error(error.str());
    }
  }
  return size;
}

std::string ToMd5String(const std::vector<uint8_t>& md5)
{
  std::ostringstream temp;
  for (auto byte : md5) {
    temp << std::hex << std::setw(2) << std::setfill('0') <<  std::uppercase <<  static_cast<int>(byte);
  };
  return temp.str();
}



bool IBlock::IsBigEndian() const {
  return byte_order_ != 0;
}

size_t IBlock::ReadHeader3(std::FILE *file) {
  file_position_ = GetFilePosition(file);
  size_t bytes = ReadStr(file, block_type_, 2);
  bytes += ReadNumber(file, block_size_);
  block_length_ = block_size_;
  return bytes;
}

size_t IBlock::ReadLinks3(std::FILE* file, size_t nof_links) {
  size_t bytes = 0;
  link_list_.clear();
  for (size_t ii = 0; ii < nof_links; ++ii) {
    uint32_t link = 0;
    bytes += ReadNumber(file, link);
    link_list_.emplace_back(link);
  }
  link_count_ = link_list_.size();
  return bytes;
}

size_t IBlock::ReadHeader4(std::FILE *file) {
  file_position_ = GetFilePosition(file);
  size_t bytes = ReadStr(file, block_type_, 4);
  uint32_t reserved = 0;
  bytes += ReadNumber(file, reserved);
  bytes += ReadNumber(file, block_length_);
  bytes += ReadNumber(file, link_count_);

  block_size_ = static_cast<uint16_t> (block_length_);

  link_list_.clear();
  for (uint64_t ii = 0; ii < link_count_; ++ii) {
    int64_t link = 0;
    bytes += ReadNumber(file, link);
    link_list_.emplace_back(link);
  }
  return bytes;
}

void IBlock::Init(const IBlock &id_block) {
  byte_order_ = id_block.byte_order_;
  version_ = id_block.version_;
}

std::size_t IBlock::ReadBool(std::FILE *file, bool &dest) const {
  uint16_t temp = 0;
  auto bytes = ReadNumber(file, temp);
  dest = temp != 0;
  return bytes;
}

std::size_t IBlock::WriteBool(std::FILE *file, bool value) const {
  const uint16_t temp = value ? 1 : 0;
  return WriteNumber(file, temp);
}

void IBlock::ReadMdComment(std::FILE *file, size_t index_md) {
  if (!md_comment_ && Link(index_md) > 0) {
    md_comment_ = std::move(std::make_unique<Md4Block>());
    md_comment_->Init(*this);
    SetFilePosition(file, Link(index_md));
    md_comment_->Read(file);
  }
}

std::string IBlock::ReadTx3(std::FILE *file, size_t index_tx) const {
  if (Link(index_tx) > 0) {
    SetFilePosition(file, Link(index_tx));
    Tx3Block tx;
    tx.Init(*this);
    tx.Read(file);
    return tx.Text();
  }
  return {};
}

std::string IBlock::ReadTx4(std::FILE *file, size_t index_tx) const {
  if (Link(index_tx) > 0) {
    SetFilePosition(file, Link(index_tx));
    Tx4Block tx;
    tx.Init(*this);
    tx.Read(file);
    return tx.Text();
  }
  return {};
}

const Md4Block *IBlock::Md4() const {
  return !md_comment_ ? nullptr : dynamic_cast<const Md4Block *> (md_comment_.get());
}

void IBlock::Md4(const std::string &xml) {
  auto md4 = std::make_unique<Md4Block>(xml);
  md_comment_ = std::move(md4);
}

std::string IBlock::Comment() const {
    if (!md_comment_) {
      return {};
    }
    const auto* md = dynamic_cast<const Tx4Block*>(md_comment_.get());
    return md != nullptr ? md->TxComment() : "";
}

const IBlock *IBlock::Find(fpos_t index) const {
  if (file_position_ == index) {
    return this;
  }
  if (md_comment_ && md_comment_->FilePosition() == index) {
    return md_comment_.get();
  }
  return nullptr;
}

void IBlock::GetBlockProperty(BlockPropertyList &dest) const {
  dest.emplace_back("Header", "", "",BlockItemType::HeaderItem);
  dest.emplace_back("Address", ToHexString(FilePosition()), "", BlockItemType::LinkItem );
  dest.emplace_back("Type", BlockType());
  dest.emplace_back("Length", std::to_string(block_length_));
  dest.emplace_back("Links", std::to_string(link_count_));
  dest.emplace_back("", "", "",BlockItemType::BlankItem);
}

std::string IBlock::MdText() const {
  if (!md_comment_) {
    return std::string();
  }
  const auto* tx = dynamic_cast<const Tx4Block*>(md_comment_.get());
  return tx == nullptr ? std::string() : tx->Text();
}

size_t IBlock::Write(std::FILE *file) {
  if (file == nullptr) {
    throw std::runtime_error("File pointer is null");
  }
  SetLastFilePosition(file);
  file_position_ = GetFilePosition(file);

  size_t bytes = 0;
  switch (block_type_.size()) {
    case 4: {
      bytes += WriteStr(file,block_type_,4);
      const std::vector<uint8_t> reserved(4, 0);
      bytes += WriteByte(file, reserved);
      bytes += WriteNumber(file, block_length_);
      uint64_t link_count = link_list_.size();
      bytes += WriteNumber(file, link_count);
      for (auto link : link_list_) {
        bytes += WriteNumber(file, link);
      }
      break;
    }

    case 2: { // MDF3 block
      bytes += WriteStr(file, block_type_,2);
      bytes += WriteNumber(file, block_size_);
      link_count_ = link_list_.size();
      for (auto link : link_list_) {
        const auto link32 = static_cast<uint32_t>(link);
        bytes += WriteNumber(file, link32);
      }
      break;
    }

    case 0:
    default: {
      std::ostringstream error;
      error << "Invalid block type. Type: " << block_type_;
      throw std::runtime_error(error.str());
    }
  }

  return bytes;
}

size_t IBlock::Update(std::FILE *file) {
  if (file == nullptr) {
    throw std::runtime_error("File pointer is null");
  }

  SetFilePosition(file, FilePosition());
  return  IsMdf4() ?
      StepFilePosition(file, 24 + (link_count_ * sizeof(int64_t))) :
      StepFilePosition(file, 4 + (link_count_ * sizeof(uint32_t)));
}

void IBlock::SetLastFilePosition(std::FILE *file) const {
#if (MSVC)
  _fseeki64(&file, 0, SEEK_END);
#else
  fseeko64(file, 0, SEEK_END);
#endif
  if (IsMdf4()) { // Well if its MDF4 file
    // Fill to align with 64 bits alignment
    for ( auto pos = GetFilePosition(file); (pos % 8) != 0; pos = GetFilePosition(file) ) {
      constexpr unsigned char fill = 0;
      fwrite( &fill, 1, 1, file );
    }
  }
}

bool IBlock::IsMdf4() const {
  return !block_type_.empty() && block_type_[0] == '#';
}

void IBlock::UpdateBlockSize(std::FILE *file, size_t bytes) {
  auto pos = FilePosition();
  if (pos <= 0) {
    throw std::runtime_error("Invalid file position");
  }
  if (file == nullptr) {
    throw std::runtime_error("File pointer is null" );
  }

  if (IsMdf4()) {
    if (block_length_ != static_cast<uint64_t>(bytes)) {
      block_length_ = static_cast<uint64_t>(bytes);
      pos += 8;
      SetFilePosition(file, pos);
      WriteNumber(file, block_length_);
    }
  } else {
    if (block_size_ != static_cast<uint32_t>(bytes)) {
      block_size_ = static_cast<uint32_t>(bytes);
      pos += 2;
      SetFilePosition(file, pos);
      WriteNumber(file, block_size_);
    }
  }
}

void IBlock::UpdateLink(std::FILE *file, size_t link_index, int64_t link) {
  if (file == nullptr) {
    throw std::runtime_error("File pointer is null" );
  }
  if (link_index >= link_list_.size()) {
    throw std::runtime_error("index is out of bounds" );
  }
  if (link == link_list_[link_index]) {
    return;
  }

  auto pos = FilePosition();
  if (pos <= 0) {
    throw std::runtime_error("Invalid file position");
  }
  if (IsMdf4()) {
    pos += 8 + (link_index * 8);
    SetFilePosition(file, pos);
    WriteNumber(file, link);
  } else {
    pos += 4 + (link_index * 4);
    uint32_t link32 = static_cast<uint32_t>(link);
    SetFilePosition(file, pos);
    WriteNumber(file, link32);
  }
  link_list_[link_index] = link;
}

}
