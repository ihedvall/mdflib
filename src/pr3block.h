/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <cstdio>
#include "iblock.h"

namespace mdf::detail {
class Pr3Block : public IBlock {
 public:
  Pr3Block() = default;
  explicit Pr3Block(const std::string& meta_data);

  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
  [[nodiscard]] std::string Text() const {
    return text_;
  }
 private:
  std::string text_;
};
}

