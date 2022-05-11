/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <limits>
#include "cc3block.h"
#include "tx3block.h"
namespace {

std::string MakeConversionTypeString(uint16_t type) {
  switch (type) {
    case 0: return "Linear";
    case 1: return "Tabular Interpolation";
    case 2: return "Tabular";
    case 6: return "Polynomial";
    case 7: return "Exponential";
    case 8: return "Logarithmic";
    case 9: return "Rational";
    case 10: return "Formula";
    case 11: return "Text Table";
    case 12: return "Text Range Table";
    case 132: return "Date";
    case 133: return "Time";
    case 0xFFFF: return "No Conversion";

    default:
      break;
  }
  return "Unknown";
}

}
namespace mdf::detail {

int64_t Cc3Block::Index() const {
  return FilePosition();
}

std::string Cc3Block::Name() const {
  return {};
}

std::string Cc3Block::Description() const {
  return {};
}

bool Cc3Block::IsUnitValid() const {
  return true;
}

std::string Cc3Block::Unit() const {
  return unit_;
}

void Cc3Block::Unit(const std::string &unit) {
  unit_ = unit;
}

void Cc3Block::Type(ConversionType type) {
  switch (type) {
    case ConversionType::Linear:
    conversion_type_ = 0;
    break;

    case ConversionType::ValueToValueInterpolation:
      conversion_type_ = 1;
      break;

    case ConversionType::ValueToValue:
      conversion_type_ = 2;
      break;

    case ConversionType::Polynomial:
      conversion_type_ = 6;
      break;

    case ConversionType::Exponential:
      conversion_type_ = 7;
      break;

    case ConversionType::Logarithmic:
      conversion_type_ = 8;
      break;

    case ConversionType::Rational:
      conversion_type_ = 9;
      break;

    case ConversionType::Algebraic:
      conversion_type_ = 10;
      break;

    case ConversionType::ValueToText:
      conversion_type_ = 11;
      break;

    case ConversionType::ValueRangeToText:
    conversion_type_ = 12;
    break;

    case ConversionType::DateConversion:
      conversion_type_ = 132;
      break;

    case ConversionType::TimeConversion:
      conversion_type_ = 133;
      break;

    case ConversionType::NoConversion:
    default:
      conversion_type_ = 0xFFFF;
      break;
  }
}

ConversionType Cc3Block::Type() const {
  switch (conversion_type_) {
    case 0: return ConversionType::Linear;
    case 1: return ConversionType::ValueToValueInterpolation;
    case 2: return ConversionType::ValueToValue;
    case 6: return ConversionType::Polynomial;
    case 7: return ConversionType::Exponential;
    case 8: return ConversionType::Logarithmic;
    case 9: return ConversionType::Rational;
    case 10: return ConversionType::Algebraic;
    case 11: return ConversionType::ValueToText;
    case 12: return ConversionType::ValueRangeToText;
    case 132: return ConversionType::DateConversion;
    case 133: return ConversionType::TimeConversion;

    case 65535 :
    default:
      break;
  }
  return ConversionType::NoConversion;
}

bool Cc3Block::IsDecimalUsed() const {
  return false;
}

uint8_t Cc3Block::Decimals() const {
  auto max = static_cast<uint8_t>(std::numeric_limits<double>::max_digits10);
  return max;
}

void Cc3Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Range Valid", range_valid_ ? "True" : "False");
  dest.emplace_back("Range Min", util::string::FormatDouble(min_, 6));
  dest.emplace_back("Range Max", util::string::FormatDouble(max_, 6));
  dest.emplace_back("Unit", unit_);

  dest.emplace_back("Conversion Type", std::to_string(conversion_type_), MakeConversionTypeString(conversion_type_));
  dest.emplace_back("Nof Values", std::to_string(nof_values_));
  if (!formula_.empty()) {
    dest.emplace_back("Formula", formula_);
  }
  for (auto par : value_list_) {
    dest.emplace_back("Parameter", util::string::FormatDouble(par, 6));
  }
  for (const auto& conv : text_conversion_list_) {
    dest.emplace_back("Parameter", util::string::FormatDouble(conv.value, 6));
    dest.emplace_back("Text", conv.text);
  }
  for (const auto& range : text_range_conversion_list_) {
    dest.emplace_back("Lower", util::string::FormatDouble(range.lower, 6));
    dest.emplace_back("Upper", util::string::FormatDouble(range.upper, 6));
    dest.emplace_back("Text", range.text);
  }

}



size_t Cc3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadBool(file, range_valid_);
  bytes += ReadNumber(file, min_);
  bytes += ReadNumber(file, max_);
  bytes += ReadStr(file, unit_, 20);
  bytes += ReadNumber(file, conversion_type_);
  if (bytes < block_size_) {
    bytes += ReadNumber(file, nof_values_);
  }

  value_list_.clear();
  formula_.clear();
  text_conversion_list_.clear();
  text_range_conversion_list_.clear();
  if (nof_values_ > 0) {
    switch (conversion_type_) {
      case 0: // Parametric
      case 6: // Polynomial
      case 7: // Exponential
      case 8: // Logarithmic
      case 9: // Rational
        for (uint16_t par = 0; par < nof_values_; ++par) {
          double temp = 0;
          bytes += ReadNumber(file, temp);
          value_list_.emplace_back(temp);
        }
        break;

      case 1: // Tabular interpolation
      case 2: // Tabular
        for (uint16_t par = 0; par < nof_values_; ++par) {
          double temp1 = 0;
          bytes += ReadNumber(file, temp1);
          value_list_.emplace_back(temp1);
          double temp2 = 0;
          bytes += ReadNumber(file, temp2);
          value_list_.emplace_back(temp2);
        }
        break;

      case 10: // Text formula
        bytes += ReadStr(file, formula_, nof_values_);
        break;

      case 11: // Text Table
        for (uint16_t par = 0; par < nof_values_; ++par) {
          TextConversion cc;
          bytes += ReadNumber(file, cc.value);
          bytes += ReadStr(file, cc.text, 32);
          text_conversion_list_.emplace_back(cc);
        }
        break;

      case 12: // Text Range Table
        for (uint16_t par = 0; par < nof_values_; ++par) {
          TextRangeConversion cc;
          bytes += ReadNumber(file, cc.lower);
          bytes += ReadNumber(file, cc.upper);
          bytes += ReadNumber(file, cc.link_text);
          text_range_conversion_list_.emplace_back(cc);
        }
        break;

      case 132: // Date
      case 133: // Time
      case 65535: // 1:1 conversion (used to set unit to a CN block)
      default:break;
    }
  }
  for (TextRangeConversion &conv: text_range_conversion_list_) {
    if (conv.link_text > 0) {
      Tx3Block temp;
      temp.Init(*this);
      SetFilePosition(file, conv.link_text);
      temp.Read(file);
      conv.text = temp.Text();
    }
  }
  return bytes;
}

size_t Cc3Block::Write(std::FILE *file) {
  const bool update = FilePosition() > 0; // Write or update the values inside the block
  if (update) {
    return block_size_;
  }
    // If text range conversion is in use, save the TX blocks first before writing the links to them.
  for (auto &range: text_range_conversion_list_) {
    uint32_t link = 0;
    if (!range.text.empty()) {
      Tx3Block temp;
      temp.Init(*this);
      temp.Write(file);
      link = static_cast<uint32_t>(temp.FilePosition());
    }
    range.link_text = link;
  }
  block_type_ = "CC";
  block_size_ = (2 + 2) + 2 + 2*8 + 20 + 2 + 2; // Will be updated at the end of the block write
  link_list_.clear();

  size_t bytes = IBlock::Write(file);
  bytes += WriteBool(file, range_valid_);
  bytes += WriteNumber(file, min_);
  bytes += WriteNumber(file, max_);
  bytes += WriteStr(file, unit_, 20);
  bytes += WriteNumber(file, conversion_type_);

  switch (conversion_type_) {
    case 0: // Parametric
    case 6: // Polynomial
    case 7: // Exponential
    case 8: // Logarithmic
    case 9: // Rational
      nof_values_ = static_cast<uint16_t>(value_list_.size());
      bytes += WriteNumber(file, nof_values_);
      for (auto par : value_list_) {
        bytes += WriteNumber(file, par);
      }
      break;

    case 1: // Tabular interpolation
    case 2: // Tabular
      nof_values_ = static_cast<uint16_t>(value_list_.size() / 2);
      bytes += WriteNumber(file, nof_values_);
      for (auto par : value_list_) {
        bytes += WriteNumber(file, par);
      }
      break;

    case 10: // Text formula
      nof_values_ = static_cast<uint16_t>(formula_.size());
      bytes += WriteNumber(file, nof_values_);
      WriteStr(file, formula_, formula_.size());
      break;

    case 11: // Text Table
      nof_values_ = static_cast<uint16_t>(text_conversion_list_.size());
      bytes += WriteNumber(file, nof_values_);
      for (const auto& conv : text_conversion_list_) {
        bytes += WriteNumber(file, conv.value);
        bytes += WriteStr(file, conv.text, 32);
      }
      break;

    case 12: // Text Range Table
      nof_values_ = static_cast<uint16_t>(text_range_conversion_list_.size());
      bytes += WriteNumber(file, nof_values_);
      for (const auto& conv : text_range_conversion_list_) {
        bytes += WriteNumber(file, conv.lower);
        bytes += WriteNumber(file, conv.upper);
        bytes += WriteNumber(file, conv.link_text);
      }
      break;

    case 132: // Date
    case 133: // Time
    case 65535: // 1:1 conversion (used to set unit to a CN block)
    default:
      nof_values_ = 0;
      bytes += WriteNumber(file, nof_values_);
      break;
  }
  UpdateBlockSize(file,bytes);
  return bytes;
}

bool Cc3Block::ConvertValueToText(double channel_value, std::string &eng_value) const {
  for (const auto& conv : text_conversion_list_) {
    if (channel_value == conv.value) {
      eng_value = conv.text;
      return true;
    }
  }
  return false;
}

bool Cc3Block::ConvertValueRangeToText(double channel_value, std::string &eng_value) const {
  if (text_range_conversion_list_.empty()) {
    return false;
  }
  for (size_t index = 1; index < text_range_conversion_list_.size(); ++index) {
    const auto& range = text_range_conversion_list_[index];
    const double key_min = range.lower;
    const double key_max = range.upper;
    if (IsChannelInteger() && channel_value >= key_min && channel_value <= key_max) {
      eng_value = range.text;
      return true;
    }

    if (IsChannelFloat() && channel_value >= key_min && channel_value < key_max) {
      eng_value = range.text;
      return true;
    }
  }
  eng_value = text_range_conversion_list_[0].text;
  return true;
}

}
