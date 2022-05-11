/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>
#include <string>
#include <limits>
#include "iblock.h"
#include "mdf/ichannelconversion.h"
#include "md4block.h"

namespace mdf::detail {


 class Cc4Block : public IBlock, public IChannelConversion {
 public:
  using RefList = std::vector<std::unique_ptr<IBlock>>;

  [[nodiscard]] virtual int64_t Index() const {
    return FilePosition();
  };

  [[nodiscard]] std::string Name() const override {
    return name_;
  }
   void Unit(const std::string &unit) override;
   [[nodiscard]] std::string Unit() const override;

  [[nodiscard]] std::string Description() const override {
     return Comment();
  }
   void Type(ConversionType type) override;
   [[nodiscard]] ConversionType Type() const override {
     return static_cast<ConversionType>(type_);
  }

  [[nodiscard]] bool IsUnitValid() const override;
  [[nodiscard]] bool IsDecimalUsed() const override {
     return flags_ & 0x01;
  }

  [[nodiscard]] uint8_t Decimals() const override {
    auto max = static_cast<uint8_t>( channel_data_type_ == 4 ?
                                     std::numeric_limits<float>::max_digits10 :
                                     std::numeric_limits<double>::max_digits10);
    return std::min(precision_, max);
  }

  [[nodiscard]] const Cc4Block* Cc() const {
    return cc_block_.get();
  }
  [[nodiscard]] const RefList& References() const {
    return ref_list_;
  }

  [[nodiscard]] const IBlock* Find(fpos_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;

 protected:
  bool ConvertValueToText(double channel_value, std::string& eng_value) const override;
  bool ConvertValueRangeToText(double channel_value, std::string& eng_value) const override;
  bool ConvertTextToValue(const std::string& channel_value, double& eng_value) const override;
  bool ConvertTextToTranslation(const std::string& channel_value, std::string& eng_value) const override;
 private:
  uint8_t type_ = 0;
  uint8_t precision_ = 0;
  uint16_t flags_ = 0;
  uint16_t nof_references_ = 0;

  double range_min_ = 0;
  double range_max_ = 0;

  std::string name_;
  std::unique_ptr<Cc4Block> cc_block_; ///< Inverse conversion block
  std::unique_ptr<Md4Block> unit_;
  RefList ref_list_;



};
}



