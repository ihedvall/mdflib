/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>
#include <memory>
#include "iblock.h"
#include "mdf/iattachment.h"
#include "md4block.h"


namespace mdf::detail {
namespace At4Flags {
constexpr uint16_t kEmbeddedData = 0x01;
constexpr uint16_t kCompressedData = 0x02;
constexpr uint16_t kUsingMd5 = 0x04;
}
class At4Block : public IBlock, public IAttachment {
 public:
  void FileName(const std::string& filename) override;
  [[nodiscard]] const std::string& FileName() const override;

  void FileType(const std::string& file_type) override;
  [[nodiscard]] const std::string& FileType() const override;

  void CreatorIndex(uint16_t creator) override;
  [[nodiscard]] uint16_t CreatorIndex() const override;

  void IsEmbedded(bool embed) override;
  [[nodiscard]] bool IsEmbedded() const override;

  void IsCompressed(bool compress) override;
  [[nodiscard]] bool IsCompressed() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  void ReadData(std::FILE *file, const std::string& dest_file) const;
  size_t Write(std::FILE *file) override;
  std::optional<std::string> Md5() override;

 private:

  uint16_t flags_ = 0;
  uint16_t creator_index_ = 0;
  // 4 byte reserved
  std::vector<uint8_t> md5_; ///< 16 byte MD5 checksum
  uint64_t original_size_ = 0;
  uint64_t nof_bytes_ = 0;

  fpos_t data_position_ = 0; ///< File position of the data BLOB
  std::string filename_;
  std::string file_type_;

};
}
