/*
 * Copyright 2024 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <vector>
#include <set>
#include "dg4block.h"
#include "dg3block.h"

namespace mdf::detail {

class ReadCache {
 public:
  ReadCache() = delete;
  ReadCache( DataListBlock* data_block, FILE* file);

  bool ParseRecord();
 private:

  FILE* file_;
  DataListBlock* data_list_;
  Dg4Block* dg4_block_ = nullptr;

  size_t file_index_ = 0;
  std::vector<uint8_t> file_buffer_; // Needs a file buffer to handle the DZ block.
  size_t data_size_ = 0;

  uint64_t data_count_ = 0;
  uint64_t max_data_count_ = 0;

  size_t block_index_ = 0;
  std::vector<DataBlock*> block_list_;
  std::set<uint64_t> record_id_list_;


  uint64_t ParseRecordId();
  bool GetNextByte(uint8_t& input);
  void GetArray(std::vector<uint8_t>& buffer);
  void SkipBytes(size_t nof_skip);
  bool SkipByte();
};


} // End namespace mdf::detail

