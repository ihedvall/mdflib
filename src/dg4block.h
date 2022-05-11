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
#include "cg4block.h"


namespace mdf::detail {
class Dg4Block : public DataListBlock, public IDataGroup {
 public:
  using Cg4List = std::vector<std::unique_ptr<Cg4Block>>;

  [[nodiscard]] int64_t Index() const override {
    return FilePosition();
  }

  [[nodiscard]] std::string Description() const override {
    return MdText();
  }

  [[nodiscard]] std::vector<IChannelGroup*> ChannelGroups() const override;

  [[nodiscard]] const Cg4List& Cg4() const {
    return cg_list_;
  }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  const IBlock* Find(fpos_t index) const override;

  size_t Read(std::FILE *file) override;
  void ReadCgList(std::FILE* file);

  void ReadData(std::FILE* file) const;
 private:
  uint8_t rec_id_size_ = 0;
  /* 7 byte reserved */
  Cg4List cg_list_;

  void ParseDataRecords(std::FILE* file, size_t nof_data_bytes) const;
  size_t ReadRecordId(std::FILE* file, uint64_t& record_id) const;
  const Cg4Block* FindCgRecordId(const uint64_t record_id) const;

};

}




