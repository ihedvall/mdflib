/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>
#include "iblock.h"

namespace mdf::detail {
struct Dependency {
  uint32_t link_dg = 0;
  uint32_t link_cg = 0;
  uint32_t link_cn = 0;
};

class Cd3Block : public IBlock {
 public:
  size_t Read(std::FILE *file) override;
 private:
  uint16_t dependency_type_ = 0;
  uint16_t nof_dependencies_ = 0;

  std::vector<Dependency> dependency_list_;
  std::vector<uint16_t> dimension_list_;
};
}
