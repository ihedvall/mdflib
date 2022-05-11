/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include "tx4block.h"
namespace mdf::detail {
class Md4Block : public Tx4Block {
 public:
  explicit Md4Block(const std::string& text);
  Md4Block() = default;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] std::string TxComment() const override;
};
}

