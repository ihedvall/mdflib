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
  [[nodiscard]] std::string FileName() const override {
    return filename_;
  }

  [[nodiscard]] std::string FileType() const override {
    return file_type_;
  }

  [[nodiscard]] bool IsEmbedded() const override {
    return flags_ & At4Flags::kEmbeddedData;
  }

  [[nodiscard]] bool IsCompressed() const override {
    return flags_ & At4Flags::kCompressedData;
  }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  void ReadData(std::FILE *file, const std::string& dest_file) const;

 private:

  uint16_t flags_ = 0;
  uint16_t creator_index_ = 0;
  // 4 byte reserved
  std::vector<uint8_t> md5_; ///< 16 byte MD5 checksum
  uint64_t original_size_ = 0;
  uint64_t nof_bytes_ = 0;

  fpos_t data_position_ = 0; ///< Stores the file position of the data BLOB
  std::string filename_;
  std::string file_type_;

};
}
