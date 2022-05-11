/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datablock.h"

namespace mdf::detail {
class Rd4Block : public DataBlock {
 public:
  size_t Read(std::FILE *file) override;
 protected:
  size_t DataSize() const override;
 private:

};
}



