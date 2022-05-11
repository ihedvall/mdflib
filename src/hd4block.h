/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdio>
#include "mdf/iheader.h"
#include "iblock.h"
#include "dg4block.h"
#include "fh4block.h"
#include "ch4block.h"
#include "at4block.h"
#include "ev4block.h"
#include "md4block.h"
#include "mdf4timestamp.h"

namespace mdf::detail {

enum class Hd4TimeClass : uint8_t {
  LocalPcTime = 0,
  ExternalTime = 10,
  ExternalNTP = 16
};

namespace Hd4Flags {
  constexpr uint8_t kStartAngleValid = 0x01;
  constexpr uint8_t kStartDistanceValid = 0x02;
}

class Hd4Block : public IBlock, public IHeader {
 public:
  using Dg4List = std::vector<std::unique_ptr<Dg4Block>>;
  using Fh4List = std::vector<std::unique_ptr<Fh4Block>>;
  using Ch4List = std::vector<std::unique_ptr<Ch4Block>>;
  using At4List = std::vector<std::unique_ptr<At4Block>>;
  using Ev4List = std::vector<std::unique_ptr<Ev4Block>>;

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

  void MeasurementId(const std::string& uuid) override;
  [[nodiscard]] std::string MeasurementId() const override;

  void RecorderId(const std::string& uuid) override;
  [[nodiscard]] std::string RecorderId() const override;

  void RecorderIndex(int64_t index) override;
  [[nodiscard]] int64_t RecorderIndex() const override;

  void StartTime(uint64_t ns_since_1970) override;
  [[nodiscard]] uint64_t StartTime() const override;

  void MetaData(const std::string &meta_data) override;
  [[nodiscard]] std::string MetaData() const override;
  [[nodiscard]] std::vector<IDataGroup *> DataGroups() const override;
  [[nodiscard]] IDataGroup *LastDataGroup() const override;
  [[nodiscard]] const Dg4List &Dg4() const {
    return dg_list_;
  }

  [[nodiscard]] const Fh4List &Fh4() const {
    return fh_list_;
  }

  [[nodiscard]] const Ch4List &Ch4() const {
    return ch_list_;
  }

  [[nodiscard]] const At4List &At4() const {
    return at_list_;
  }

  [[nodiscard]] const Ev4List &Ev4() const {
    return ev_list_;
  }

  [[nodiscard]] const IBlock* Find(fpos_t index) const override;
  void GetBlockProperty(BlockPropertyList &dest) const override;

  size_t Read(std::FILE *file) override;
  void ReadMeasurementInfo(std::FILE *file);
  void ReadEverythingButData(std::FILE *file);

 private:
  Mdf4Timestamp timestamp_;

  uint8_t time_class_ = 0;
  uint8_t flags_ = 0;
  /* 1 byte reserved */;
  double start_angle_ = 0;    ///< Unit is radians.
  double start_distance_ = 0; ///< Unit is meters.

  Dg4List dg_list_;
  Fh4List fh_list_;
  Ch4List ch_list_;
  At4List at_list_;
  Ev4List ev_list_;

};
}


