/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <memory>
#include <vector>
#include "mdf/idatagroup.h"
#include "datalistblock.h"
#include "tr3block.h"
#include "cg3block.h"
#include "dt3block.h"

namespace mdf::detail {

class Dg3Block : public DataListBlock, public IDataGroup {
 public:
  using Cg3List = std::vector<std::unique_ptr<Cg3Block>>;

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string Description() const override;
  [[nodiscard]] std::vector<IChannelGroup*> ChannelGroups() const override;


  void AddCg3(std::unique_ptr<Cg3Block>& cg3);
  [[nodiscard]] const Cg3List& Cg3() const;
  [[nodiscard]] const Tr3Block* Tr3() const {
    return tr_block_.get();
  }
  uint16_t NofRecordId() const {
    return nof_record_id_;
  }
  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] const IBlock* Find(fpos_t index) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  void ReadData(std::FILE* file) const;
 private:

  uint16_t nof_cg_blocks_ = 0;
  uint16_t nof_record_id_ = 0;

  std::unique_ptr<Tr3Block> tr_block_;
  Cg3List cg_list_;
  void ParseDataRecords(std::FILE* file, size_t nof_data_bytes) const;
  const Cg3Block* FindCgRecordId(const uint64_t record_id) const;
};

}




