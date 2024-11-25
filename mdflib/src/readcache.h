/*
 * Copyright 2024 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <vector>
#include <set>
#include <map>
#include <functional>

#include "dg4block.h"
#include "dg3block.h"

namespace mdf::detail {

class ReadCache {
 public:
  ReadCache() = delete;
  ReadCache( MdfBlock* block, std::streambuf& buffer);

  bool ParseRecord();
  bool ParseRangeRecord(DgRange& range);
  bool ParseVlsdCgData();
  bool ParseSignalData(); // Used against SD/DL/DZ blocks

  void SetRecordId(uint64_t record_id) {
    record_id_list_.clear();
    record_id_list_.insert(record_id);
  }

 void SetOffsetFilter(const std::vector<uint64_t>& offset_list);

 void SetCallback(const std::function<void(uint64_t, const std::vector<uint8_t>&)>& callback );
 private:

  std::streambuf& buffer_;
  DataListBlock* data_list_ = nullptr; ///< List of data_block example a DL block
  DataBlock* data_block_ = nullptr; ///< A single data block
  Dg4Block* dg4_block_ = nullptr;

  size_t file_index_ = 0;
  std::vector<uint8_t> file_buffer_; // Needs a file buffer to handle the DZ block.
  uint64_t data_size_ = 0;

  uint64_t data_count_ = 0;
  uint64_t max_data_count_ = 0;

  size_t block_index_ = 0;
  std::vector<DataBlock*> block_list_;
  std::set<uint64_t> record_id_list_;
  std::map<uint64_t, const Cg4Block*> available_cg_list_;

  uint64_t offset_ = 0;
  std::set<uint64_t> offset_filter_;
  std::function<void(uint64_t, const std::vector<uint8_t>&)> callback_;

  uint64_t ParseRecordId();
  bool GetNextByte(uint8_t& input);
  void GetArray(std::vector<uint8_t>& buffer);
  void SkipBytes(size_t nof_skip);
  bool SkipByte();
};


} // End namespace mdf::detail

