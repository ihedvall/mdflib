/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
namespace mdf::detail {

class Si4Block : public IBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;

  std::string Name() const {
    return name_;
  }
  std::string Path() const {
    return path_;
  }
 private:
  uint8_t type_ = 0;
  uint8_t bus_type_ = 0;
  uint8_t flags_ = 0;
  /* 5 byte reserved */
  std::string name_;
  std::string path_;

};
}





