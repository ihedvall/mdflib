/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>

#include "datalistblock.h"

namespace mdf::detail {
class Sr3Block : public DataListBlock {
 public:
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

 private:
  uint32_t nof_reduced_samples_ = 0;
  double time_interval_ = 0;
};

}  // namespace mdf::detail
