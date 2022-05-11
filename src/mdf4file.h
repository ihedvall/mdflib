/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>
#include "mdf/mdffile.h"
#include "idblock.h"
#include "hd4block.h"

namespace mdf::detail {
class Mdf4File : public MdfFile {
 public:
  explicit Mdf4File(std::unique_ptr<IdBlock> id_block);
  ~Mdf4File() override = default;

  void Attachments(AttachmentList& dest) const override;
  void DataGroups(DataGroupList& dest ) const override;
  [[nodiscard]] std::string Version() const override;
  [[nodiscard]] std::string ProgramId() const override;
  [[nodiscard]] IHeader* Header() const override;

  [[nodiscard]] const IBlock* Find(fpos_t id) const ;

  [[nodiscard]] bool IsMdf4() const override;


  void ReadHeader(std::FILE *file) override;
  void ReadMeasurementInfo(std::FILE *file) override;
  void ReadEverythingButData(std::FILE *file) override;

  [[nodiscard]] const IdBlock &Id() const;
  [[nodiscard]] const Hd4Block &Hd() const;

  Mdf4File() = delete;
  Mdf4File(const Mdf4File &) = delete;
  Mdf4File(Mdf4File &&) = delete;
  Mdf4File &operator=(const Mdf4File &) = delete;
  Mdf4File &operator=(Mdf4File &&) = delete;
 private:
  std::unique_ptr<IdBlock> id_block_;
  std::unique_ptr<Hd4Block> hd_block_;
};
}


