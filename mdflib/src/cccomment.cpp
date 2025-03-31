/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/cccomment.h"

namespace mdf {

CcComment::CcComment()
: MdComment("CC") {

}

const HoCompuMethod& CcComment::CompuMethod() const {
  return method_;
}

HoCompuMethod& CcComment::CompuMethod() {
  return method_;
}

}  // namespace mdf