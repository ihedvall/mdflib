/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"
#include "mdf/isourceinformation.h"
namespace mdf::detail {

class Si4Block : public IBlock, public ISourceInformation {
 public:

  [[nodiscard]] int64_t Index() const override;

  void Name(const std::string &name) override;
  [[nodiscard]] const std::string& Name() const override;

  void Path(const std::string &path) override;
  [[nodiscard]] const std::string& Path() const override;

  void Description(const std::string &desc) override;
  [[nodiscard]] std::string Description() const override;

  void Type(SourceType type) override;
  [[nodiscard]] SourceType Type() const override;

  void Bus(BusType type) override;
  [[nodiscard]] BusType Bus() const override;

  void Flags(uint8_t flags) override;
  [[nodiscard]] uint8_t Flags() const override;

  IMetaData *MetaData() override;
  [[nodiscard]] const IMetaData *MetaData() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
 private:
  uint8_t type_ = 0;
  uint8_t bus_type_ = 0;
  uint8_t flags_ = 0;
  /* 5 byte reserved */
  std::string name_;
  std::string path_;

};
}





