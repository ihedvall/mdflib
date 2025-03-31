/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdComment.h"
#include "mdf/hocompumethod.h"

namespace mdf {

class CcComment : public MdComment {
 public:
  CcComment();

  [[nodiscard]] const HoCompuMethod& CompuMethod() const;
  [[nodiscard]] HoCompuMethod& CompuMethod();

 private:
  HoCompuMethod method_;
};

}  // namespace mdf

