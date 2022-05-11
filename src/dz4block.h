/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datablock.h"

namespace mdf::detail {
enum class Dz4ZipType : uint8_t {
  Deflate = 0,
  TransposeAndDeflate = 1
};

class Dz4Block : public DataBlock {
 public:
  std::string OrigBlockType() const {
    return orig_block_type_;
  }

  size_t DataSize() const override {
    return orig_data_length_;
  }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  size_t CopyDataToFile(std::FILE *from_file, std::FILE *to_file) const override;
  size_t CopyDataToBuffer(std::FILE* from_file, std::vector<uint8_t>& buffer, size_t& buffer_index) const override;

 private:
  std::string orig_block_type_;
  uint8_t type_ = 0;
  /* 1 byte reserved */
  uint32_t parameter_ = 0;
  uint64_t orig_data_length_ = 0;
  uint64_t data_length_ = 0;

};
}





