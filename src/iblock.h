/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio>

#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>

#include "blockproperty.h"

namespace mdf::detail {

class Md4Block;
using BlockPropertyList = std::vector<BlockProperty>;

std::fpos_t GetFilePosition(std::FILE *file);
void SetFilePosition(std::FILE *file, std::fpos_t position);
void SetFirstFilePosition(std::FILE *file);

size_t StepFilePosition(std::FILE* file, size_t steps);

[[nodiscard]] std::string ReadBlockType(std::FILE* file);
std::size_t ReadByte(std::FILE *file, std::vector<uint8_t> &dest, size_t size);
std::size_t WriteByte(std::FILE *file, const std::vector<uint8_t>& source);

std::size_t ReadStr(std::FILE *file, std::string &dest, size_t size);
std::size_t WriteStr(std::FILE *file, const std::string &source, size_t size);

template <typename T>
std::string ToHexString(const T& value)
{
  std::ostringstream temp;
  temp << "0x" << std::hex << std::setfill('0') <<  std::uppercase;
  if (sizeof(T) == 1) {
    temp << std::setw(2) << static_cast<int>(value);
  } else {
    temp << value;
  }
  return temp.str();
}

template <typename T>
std::string ToString(const T& value)
{
  std::ostringstream temp;
  if (sizeof(T) == 1) {
    temp << std::setw(2) << static_cast<int>(value);
  } else {
    temp << value;
  }
  return temp.str();
}
std::string ToMd5String(const std::vector<uint8_t>& md5);


class IBlock {
 public:

  virtual ~IBlock() = default;

  virtual void GetBlockProperty(BlockPropertyList& dest) const;
  [[nodiscard]] virtual const IBlock* Find(fpos_t index) const;

  [[nodiscard]] fpos_t FilePosition() const {
    return file_position_;
  }

  void SetLastFilePosition(std::FILE* file) const;

  [[nodiscard]] std::string BlockType() const {
    if (block_type_.size() == 4) {
      return block_type_.substr(2);
    }
    return block_type_;
  }

  [[nodiscard]] uint64_t BlockLength() const {
    return block_size_ + block_length_;
  }

  /**
   * Initialize the block by copying the byte order and version.
   * Note that MDF4 always uses little endian byte order.
   * @param id_block ID or parent block
   */
  virtual void Init(const IBlock &id_block);

  [[nodiscard]] bool IsBigEndian() const;

  [[nodiscard]] uint16_t Version() const {
    return version_;
  }

  [[nodiscard]] int64_t Link(size_t ii) const {
    return link_list_.size() > ii ? link_list_[ii] : 0;
  }

  [[nodiscard]] const Md4Block *Md4() const;
  void  Md4(const std::string& xml);

  virtual size_t Read(std::FILE *file) = 0;
  virtual size_t Write(std::FILE *file);
  size_t Update(std::FILE* file);
  void UpdateLink(std::FILE* file, size_t link_index, int64_t link);

  [[nodiscard]] virtual std::string Comment() const;

 protected:
  /**
   * The MDF3 has big or little endian byte order while MDF4 always uses little endian byte order.
   */
  uint16_t byte_order_ = 0; ///< Default set to Intel (little) byte order.
  uint16_t version_ = 410;  ///< Default set to 4.1.

  fpos_t file_position_ = 0;       ///< 64-bit file position.
  std::string block_type_;         ///< MDF header. MDF3 has 2 characters. MDF4 has 4 characters.
  uint16_t block_size_ = 0;     ///< MDF3 16-bit block size.
  uint64_t block_length_ = 0;   ///< MDF4 64-bit block size.
  uint64_t link_count_ = 0;     ///< MDF4 number of links.
  std::vector<int64_t> link_list_; ///< MDF link list

  std::unique_ptr<IBlock> md_comment_; ///< Most MDF4 block has a MD block referenced

  IBlock() = default;

  [[nodiscard]] bool IsMdf4() const;
  size_t ReadHeader3(std::FILE *file); ///< Reads a MDF3 block header.
  size_t ReadLinks3(std::FILE* file, size_t nof_links); ///< Reads MDF3 links into the link list.

  size_t ReadHeader4(std::FILE *file); ///< Read in MDF4 header and links.

  void ReadMdComment(std::FILE *file, size_t index_md);
  std::string ReadTx3(std::FILE *file, size_t index_tx) const;
  std::string ReadTx4(std::FILE *file, size_t index_tx) const;
  std::size_t ReadBool(std::FILE *file, bool &dest) const;
  std::size_t WriteBool(std::FILE* file, bool value) const;

  [[nodiscard]] std::string MdText() const;

  void UpdateBlockSize(std::FILE* file, size_t bytes);


  template<typename T>
  std::size_t ReadNumber(std::FILE *file, T &dest) const {
    if (IsBigEndian()) {
      boost::endian::endian_buffer<boost::endian::order::big, T, sizeof(T) * 8> buff;
      auto count = std::fread(buff.data(), sizeof(T), 1, file);
      if (count != 1) {
        throw std::ios_base::failure("Invalid number of bytes read");
      }
      dest = buff.value();
    } else {
      boost::endian::endian_buffer<boost::endian::order::little, T, sizeof(T) * 8> buff;
      auto count = std::fread(buff.data(), sizeof(T), 1, file);
      if (count != 1) {
        throw std::ios_base::failure("Invalid number of bytes read");
      }
      dest = buff.value();
    }
    return sizeof(T);
  }

  template<typename T>
  std::size_t WriteNumber( std::FILE *file, const T &source) const {
    if (file == nullptr) {
      throw std::runtime_error("File pointer is null. Invalid use of function.");
    }
    if (IsBigEndian()) {
      boost::endian::endian_buffer<boost::endian::order::big, T, sizeof(T) * 8> buff(source);
      auto count = std::fwrite(buff.data(), 1, sizeof(T),  file);
      if (count != sizeof(T)) {
        throw std::runtime_error("Invalid number of bytes written");
      }
    } else {
      boost::endian::endian_buffer<boost::endian::order::little, T, sizeof(T) * 8> buff(source);
      auto count = std::fwrite(buff.data(), 1, sizeof(T), file);
      if (count != sizeof(T)) {
        throw std::runtime_error("Invalid number of bytes written");
      }
    }
    return sizeof(T);
  }
};

}
