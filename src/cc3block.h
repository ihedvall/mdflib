/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include <cstdio>
#include "iblock.h"
#include "mdf/ichannelconversion.h"

namespace mdf::detail {

struct TextConversion {
  double value = 0;
  std::string text;
};

struct TextRangeConversion {
  double lower = 0;
  double upper = 0;
  uint32_t link_text = 0;
  std::string text;
};

class Cc3Block : public IBlock, public IChannelConversion {
 public:
  [[nodiscard]] virtual int64_t Index() const override;
  [[nodiscard]] std::string Name() const override;
  [[nodiscard]] std::string Description() const override;
  void Unit(const std::string &unit) override;
  void Type(ConversionType type) override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] bool IsUnitValid() const override;

  [[nodiscard]] ConversionType Type() const override;


  [[nodiscard]] bool IsDecimalUsed() const override;

  [[nodiscard]] uint8_t Decimals() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
 protected:
  bool ConvertValueToText(double channel_value, std::string& eng_value) const override;
  bool ConvertValueRangeToText(double channel_value, std::string& eng_value) const override;
 private:
  bool range_valid_ = false;
  double min_ = 0.0;
  double max_ = 0.0;
  std::string unit_;
  uint16_t conversion_type_ = 0xFFFF;

  std::string formula_;
  std::vector<TextConversion> text_conversion_list_;
  std::vector<TextRangeConversion> text_range_conversion_list_;

};
}
