/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>

#include "mdf/imetadata.h"
#include "mdfblock.h"

namespace mdf::detail {
std::string FixCommentToLine(const std::string& comment, size_t max);

class Tx4Block : public MdfBlock {
 public:
  Tx4Block() = default;
  explicit Tx4Block(const std::string& text);

  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] bool IsTxtBlock() const;
  size_t Read(std::FILE* file) override;
  size_t Write(std::FILE* file) override;

  void Text(const std::string& text) {text_ = text;}
  [[nodiscard]] std::string Text() const;
  [[nodiscard]] virtual std::string TxComment() const;

 protected:
  std::string text_;
};

}  // namespace mdf::detail
