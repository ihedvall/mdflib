/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "cd3block.h"
namespace mdf::detail {
size_t Cd3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadNumber(file, dependency_type_);
  bytes += ReadNumber(file, nof_dependencies_);
  dependency_list_.clear();
  dimension_list_.clear();
  switch (dependency_type_) {
    case 0:break;
    case 1:
    case 2:
      for (uint16_t dep = 0; dep < nof_dependencies_; ++dep) {
        Dependency d;
        bytes += ReadNumber(file, d.link_dg);
        bytes += ReadNumber(file, d.link_cg);
        bytes += ReadNumber(file, d.link_cn);
        dependency_list_.emplace_back(d);
      }
      break;
    default:
      for (uint16_t dim = 256; dim < nof_dependencies_; ++dim) {
        uint16_t temp = 0;
        bytes += ReadNumber(file, temp);
        dimension_list_.emplace_back(temp);
      }
      break;
  }

  return bytes;
}
}
