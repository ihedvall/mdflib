/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include "tx4block.h"

namespace mdf::detail {
class Md4Block : public Tx4Block , public IMetaData {
 public:
  explicit Md4Block(const std::string& text);
  Md4Block() = default;

  void GetBlockProperty(BlockPropertyList& dest) const override;

  void TxComment(const std::string& tx_comment);
  [[nodiscard]] std::string TxComment() const override;

  void XmlSnippet(const std::string& text) override;
  [[nodiscard]] const std::string& XmlSnippet() const override;


};
}

