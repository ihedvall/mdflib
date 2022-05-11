/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "iblock.h"

namespace mdf::detail {
class IdBlock : public IBlock {
 public:
  IdBlock();
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  [[nodiscard]] std::string FileId() const;
  [[nodiscard]] std::string VersionString() const;
  [[nodiscard]] std::string ProgramId() const;

  void SetDefaultMdf3Values();
 private:
  std::string file_identifier_ = "MDF     "; ///< Note the string must be 8 characters including spaces
  std::string format_identifier_ = "4.10";
  std::string program_identifier_ = "MdfWrite";
  /* uint16_t byte_order_ = 0; Default 0 = Little endian (Intel byte order). Defined in IBlock class. */
  uint16_t floating_point_format_ = 0; ///< Default IEEE standard
  /* uint16_t version_ = 410;   Defined in IBlock class. */
  uint16_t code_page_number_ = 0;
  /* 2 byte reserved */
  /* 26 byte reserved */
  uint16_t standard_flags_ = 0;
  uint16_t custom_flags_ = 0;

};
}



