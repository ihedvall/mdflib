/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "datablock.h"

namespace mdf::detail {

class Ri4Block : public DataBlock {
 public:
  size_t Read(std::FILE *file) override;

 protected:
  [[nodiscard]] size_t DataSize() const override;
};

}  // namespace mdf::detail
