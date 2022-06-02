/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
#include "mdf/ifilehistory.h"
#include "mdf4timestamp.h"

namespace mdf::detail {
class Fh4Block : public IBlock, public IFileHistory {
 public:
  [[nodiscard]] int64_t Index() const override;
  void Time(uint64_t ns_since_1970) override;
  [[nodiscard]] uint64_t Time() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] IMetaData* MetaData() override;
  [[nodiscard]] const IMetaData* MetaData() const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

 private:
  Mdf4Timestamp timestamp_;
};
}





