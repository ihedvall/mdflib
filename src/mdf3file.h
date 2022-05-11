/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include "mdf/idatagroup.h"
#include "mdf/mdffile.h"
#include "idblock.h"
#include "hd3block.h"

namespace mdf::detail {

class Mdf3File : public MdfFile {
 public:
  explicit Mdf3File();
  explicit Mdf3File(std::unique_ptr<IdBlock> id_block);
  ~Mdf3File() override = default;

  void Attachments(AttachmentList& dest) const override;
  void DataGroups(DataGroupList& dest ) const override;
  [[nodiscard]] std::string Version() const override;
  [[nodiscard]] std::string ProgramId() const override;
  [[nodiscard]] IHeader* Header() const override;

  [[nodiscard]] IDataGroup* CreateDataGroup();

  [[nodiscard]] const IBlock* Find(fpos_t id) const;

  [[nodiscard]] bool IsMdf4() const override;

  void ReadHeader(std::FILE *file) override;
  void ReadMeasurementInfo(std::FILE *file) override;
  void ReadEverythingButData(std::FILE *file) override;

  [[nodiscard]] const IdBlock &Id() const;
  [[nodiscard]] const Hd3Block &Hd() const;

  bool Write(std::FILE* file);

  Mdf3File(const Mdf3File &) = delete;
  Mdf3File(Mdf3File &&) = delete;
  Mdf3File &operator=(const Mdf3File &) = delete;
  Mdf3File &operator=(Mdf3File &&) = delete;
 private:
  std::unique_ptr<IdBlock> id_block_;
  std::unique_ptr<Hd3Block> hd_block_;
};

}
