/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdfblock.h"

namespace mdf::detail {
class Tx3Block : public MdfBlock {
 public:
  explicit Tx3Block(const std::string &text);
  Tx3Block() = default;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
  [[nodiscard]] std::string Text() const;

 private:
  std::string text_;
};
}  // namespace mdf::detail
