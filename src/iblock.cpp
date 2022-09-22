/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <sstream>
#include <ios>
#include <thread>
#include <chrono>
#include "mdf/mdflogstream.h"
#include <stdio.h>
#include "iblock.h"
#include "md4block.h"
#include "tx3block.h"
#include "ixmlfile.h"

using namespace std::chrono_literals;

namespace mdf::detail {

int64_t GetFilePosition(std::FILE *file) {
  const auto pos = Platform::ftell64(file);
  if (pos == -1) {
    throw std::ios_base::failure("Failed to get a file position");
  }
  return pos;
}

void SetFilePosition(std::FILE *file, int64_t position) {
  // To check if we are able to get a file position
  auto pos = GetFilePosition(file);

  // Fast check if it already is in position
  if (pos == position) {
    return;
  }

  if (Platform::fseek64(file, pos, SEEK_SET)) {
    throw std::ios_base::failure("Failed to set a file position");
  }
}

void SetFirstFilePosition(std::FILE *file) {
    SetFilePosition(file, 0);
}

size_t StepFilePosition(std::FILE *file, size_t steps) {
    Platform::fseek64(file, steps, SEEK_CUR);
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

std::size_t WriteBytes(std::FILE *file, size_t nof_bytes) {
  size_t bytes = 0;
  if (nof_bytes > 0) {
    const std::vector<uint8_t> empty_list(nof_bytes, 0);
    bytes = WriteByte(file, empty_list);
  }
  return bytes;
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
  MdfHelper::Trim(dest);
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

bool OpenMdfFile(FILE *&file, const std::string &filename, const std::string &mode) {
  if (file != nullptr) {
    fclose(file);
    file = nullptr;
  }

  for (size_t ii = 0; ii < 6'000; ++ii) {
    const auto open = Platform::fileopen(&file, filename.c_str(), mode.c_str());
    switch (open) {
      case EEXIST:
      case EACCES:
        if (file != nullptr) {
          fclose(file);
          file = nullptr;
        }
        std::this_thread::sleep_for(10ms);
        break;

      case ENOENT:
        if (file != nullptr) {
          fclose(file);
          file = nullptr;
        }
        MDF_ERROR() << "File doesn't exist. File: " << filename;
        return false;

      default:
        if (open != 0) {
          if (file != nullptr) {
            fclose(file);
            file = nullptr;
          }
          MDF_ERROR() << "Failed to open the file. File: " << filename
                      << ". Error: " << strerror(open) << " (" << open << ")";
          return false;
        }
        ii = 6000;
        break;
    }
  }
  if (file == nullptr) {
    MDF_ERROR() << "Failed to open the file due to lock timeout (5 s). File: " << filename;
  }
  return file != nullptr;

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

void IBlock::WriteMdComment(std::FILE *file, size_t index_md) {
  if (md_comment_ && Link(index_md) == 0) {
    md_comment_->Write(file);
    UpdateLink(file, index_md, md_comment_->FilePosition());
  }
}

void IBlock::WriteTx4(std::FILE *file, size_t index_tx, const std::string& text) {
  if (!text.empty() && Link(index_tx) == 0) {
    Tx4Block tx4(text);
    tx4.Init(*this);
    tx4.Write(file);
    UpdateLink(file, index_tx, tx4.FilePosition());
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

const IBlock *IBlock::Find(int64_t index) const {
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
    return {};
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
  if (IsMdf4()) {
    bytes += WriteStr(file,block_type_,4);
    const std::vector<uint8_t> reserved(4, 0);
    bytes += WriteByte(file, reserved);
    bytes += WriteNumber(file, block_length_);
    uint64_t link_count = link_list_.size();
    bytes += WriteNumber(file, link_count);
    for (auto link : link_list_) {
      bytes += WriteNumber(file, link);
    }
  } else {
    bytes += WriteStr(file, block_type_,2);
    bytes += WriteNumber(file, block_size_);
    link_count_ = link_list_.size();
    for (auto link : link_list_) {
      const auto link32 = static_cast<uint32_t>(link);
      bytes += WriteNumber(file, link32);
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
  Platform::fseek64(file, 0, SEEK_END);
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
    return; // No need to update the link
  }

  auto pos = FilePosition();

  // If the block not yet is written, the link will be written later
  if (pos > 0) {
    if (IsMdf4()) {
      pos += 24 + static_cast<int64_t>(link_index * 8);
      SetFilePosition(file, pos);
      WriteNumber(file, link);
    } else {
      pos += 4 + static_cast<int64_t>(link_index * 4);
      const auto link32 = static_cast<uint32_t>(link);
      SetFilePosition(file, pos);
      WriteNumber(file, link32);
    }
  }
  link_list_[link_index] = link;
}

void IBlock::CreateMd4Block() {
  const auto* const_me = this;
  const auto* metadata = const_me->MetaData();
  if (metadata != nullptr) {
    // Already done
    return;
  }
    // Create an MD block and optional copy the text if it was a TX block
  auto md4 = std::make_unique<Md4Block>();

  std::ostringstream root_name;
  root_name << BlockType() << "comment";

  auto xml = CreateXmlFile();
  auto& root = xml->RootName(root_name.str());

  if (md_comment_ ) {
    const auto* tx4 = dynamic_cast<const Tx4Block*> (md_comment_.get());
    xml->SetProperty("TX", tx4 != nullptr ? tx4->Text() : std::string());
  }

  md4->XmlSnippet(xml->WriteString(true));
  md_comment_ =  std::move(md4);
}

std::string IBlock::BlockType() const {
  if (block_type_.size() == 4) {
    return block_type_.substr(2);
  }
  return block_type_;
}

IMetaData *IBlock::MetaData() {
  CreateMd4Block();
  return dynamic_cast<IMetaData *>(md_comment_.get());
}

const IMetaData *IBlock::MetaData() const {
  return !md_comment_ ? nullptr : dynamic_cast<const IMetaData *>(md_comment_.get());
}

}
