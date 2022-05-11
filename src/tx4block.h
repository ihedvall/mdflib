/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include "iblock.h"

namespace mdf::detail {
std::string FixCommentToLine(const std::string& comment, size_t max);


class Tx4Block : public IBlock {
 public:
  Tx4Block() = default;
  Tx4Block(const std::string& text);

  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] bool IsTxtBlock() const;
  size_t Read(std::FILE *file) override;
  [[nodiscard]] std::string Text() const;
  [[nodiscard]] virtual std::string TxComment() const;
 protected:
  std::string text_;
};

}
