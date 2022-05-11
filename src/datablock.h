/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "iblock.h"
namespace mdf::detail {

class DataBlock : public IBlock {
 public:

  fpos_t DataPosition() const {
    return data_position_;
  }
  virtual size_t DataSize() const = 0;
  virtual size_t CopyDataToFile(std::FILE* from_file, std::FILE* to_file) const;
  virtual size_t CopyDataToBuffer(std::FILE* from_file, std::vector<uint8_t>& buffer, size_t& buffer_index) const;
 protected:
  fpos_t data_position_ = 0;
};

}



