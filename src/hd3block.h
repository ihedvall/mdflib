/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <memory>
#include <vector>
#include "iblock.h"
#include "mdf/iheader.h"
#include "pr3block.h"
#include "dg3block.h"

namespace mdf::detail {

class Hd3Block : public IBlock, public IHeader {
 public:
  using Dg3List = std::vector<std::unique_ptr<Dg3Block>>;
  Hd3Block() = default;
  ~Hd3Block() override = default;

  [[nodiscard]] int64_t Index() const override;

  void Author(const std::string &author) override;
  [[nodiscard]] std::string Author() const override;

  void Department(const std::string &department) override;
  [[nodiscard]] std::string Department() const override;

  void Project(const std::string &name) override;
  [[nodiscard]] std::string Project() const override;

  void Subject(const std::string &subject) override;
  [[nodiscard]] std::string Subject() const override;

  void Description(const std::string &description) override;
  [[nodiscard]] std::string Description() const override;

  void StartTime(uint64_t ns_since_1970) override;
  [[nodiscard]] uint64_t StartTime() const override;

  void MetaData(const std::string &meta_data) override;
  [[nodiscard]] std::string MetaData() const override;
  [[nodiscard]] std::vector<IDataGroup *> DataGroups() const override;
  [[nodiscard]] IDataGroup *LastDataGroup() const override;


  void AddDg3(std::unique_ptr<Dg3Block>& dg3);
  [[nodiscard]] const Dg3List& Dg3() const;


  [[nodiscard]] std::string Comment() const override;
  [[nodiscard]] const IBlock* Find(fpos_t index) const override;
  void GetBlockProperty(BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  void ReadMeasurementInfo(std::FILE *file);
  void ReadEverythingButData(std::FILE *file);
 private:

  uint16_t nof_dg_blocks_ = 0;
  std::string date_ = "01:01:1970";
  std::string time_ = "00:00:00";
  std::string author_;
  std::string organisation_;
  std::string project_;
  std::string subject_;
  uint64_t local_timestamp_ = 0; ///< Nanosecond since 1 Jan 1970 with DST (local time)
  int16_t dst_offset_ = 0; ///< DST offset in hours
  uint16_t time_quality_ = 0; ///< Default local PC time
  std::string timer_id_ = "Local PC Reference Time";

  std::unique_ptr<Pr3Block> pr_block_;
  Dg3List dg_list_;
  std::string comment_;

};
}


