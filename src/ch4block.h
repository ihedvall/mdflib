/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include "iblock.h"

namespace mdf::detail {
class Ch4Block : public IBlock {
 public:
  using Ch4List = std::vector<std::unique_ptr<Ch4Block>>;
  using RefList = std::vector<int64_t>;

  const Ch4List& Ch() const {
    return ch_list_;
  }

  const IBlock* Find(fpos_t index) const override;

  [[nodiscard]] int64_t Index() const {
    return FilePosition();
  }

  [[nodiscard]] std::string Name() const {
    return name_;
  }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
 private:
  uint32_t nof_elements_ = 0;
  uint8_t type_ = 0;
  std::string name_;

  Ch4List ch_list_;
  RefList ref_list;
};
}




