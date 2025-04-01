/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"

namespace mdf {

class AtComment : public MdComment {
 public:
  AtComment();
  [[nodiscard]] std::string ToXml() const;
  void FromXml(const std::string& xml_snippet);
 private:
};

}  // namespace mdf

