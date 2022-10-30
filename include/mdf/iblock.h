/*
 * * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <string>


namespace mdf {

class IBlock {
 public:
  virtual ~IBlock() = default;
  [[nodiscard]] virtual int64_t Index() const = 0;
  [[nodiscard]] virtual std::string BlockType() const = 0;
 protected:

};

}  // namespace mdf
