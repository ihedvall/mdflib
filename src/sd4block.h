/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <map>
#include "datablock.h"
#include "vlsddata.h"

namespace mdf::detail {

class Sd4Block : public DataBlock {
 public:
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  uint64_t AppendData(const std::string& text);
  uint64_t AppendData(const std::vector<uint8_t>& data);
 protected:
  [[nodiscard]] size_t DataSize() const override;
 private:
  uint64_t AppendData(const VlsdData& data);
  std::map<VlsdData, uint64_t> sorted_data_;
};
}  // namespace mdf::detail
