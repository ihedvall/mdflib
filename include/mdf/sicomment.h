/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"
#include "mdf/mdalternativename.h"
#include "mdf/mdstring.h"

namespace mdf {

class SiComment : public MdComment {
 public:
  SiComment();
  [[nodiscard]] const MdAlternativeName& Path() const;
  [[nodiscard]] MdAlternativeName& Path();

  [[nodiscard]] const MdAlternativeName& Bus() const;
  [[nodiscard]] MdAlternativeName& Bus();

  void Protocol(MdString protocol);
  [[nodiscard]] const MdString& Protocol() const;

 private:
  // names is define in MdComment
  MdAlternativeName path_;
  MdAlternativeName bus_;
  MdString protocol_;
};

}  // namespace mdf


