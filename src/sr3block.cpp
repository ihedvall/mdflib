/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <cstdio>
#include "sr3block.h"

namespace {

constexpr size_t kIndexSr = 0;
constexpr size_t kIndexData = 1;

}

namespace mdf::detail {

size_t Sr3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadLinks3(file, 2);
  bytes += ReadNumber(file, nof_reduced_samples_);
  bytes += ReadNumber(file, time_interval_);
  return bytes;
}

size_t Sr3Block::Write(std::FILE *file) {
  // SR block must be written with its data once with no updates
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  block_type_ = "SR";
  block_size_ = (2 + 2) + (2 * 4) + 4 + 8;
  link_list_.resize(2, 0);
  size_t bytes = IBlock::Write(file);
  bytes += WriteNumber(file, nof_reduced_samples_);
  bytes += WriteNumber(file, time_interval_);

  for (size_t index = 0; index < block_list_.size(); ++index) {
    auto& data = block_list_[index];
    if (!data) {
      continue;
    }
    if (data->FilePosition() > 0) {
      continue;
    }
    data->Write(file);
    if (index == 0) {
      UpdateLink(file, kIndexData, data->FilePosition());
    }
  }

  return bytes;
}

}