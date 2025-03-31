/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include "mdf/sicomment.h"

#include <utility>

namespace mdf {

SiComment::SiComment()
: MdComment("SI") {

}

const MdAlternativeName& SiComment::Path() const {
  return path_;
}

MdAlternativeName& SiComment::Path() {
  return path_;
}

const MdAlternativeName& SiComment::Bus() const {
  return bus_;
}

MdAlternativeName& SiComment::Bus() {
  return bus_;
}

void SiComment::Protocol(MdString protocol) {
  protocol_ = std::move(protocol);
}

const MdString& SiComment::Protocol() const {
  return protocol_;
}

}  // namespace mdf