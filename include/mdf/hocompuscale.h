/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <optional>
#include <vector>

#include "mdf/hoscaleconstraint.h"
#include "mdf/mdstring.h"

namespace mdf {

class HoCompuScale;

using CoeffList = std::vector<double>;
using HoCompuScaleList = std::vector<HoCompuScale>;

class HoCompuScale : public HoScaleConstraint {
 public:


 private:
  MdStringList description_list_;

  std::optional<double> const_float_value_;
  std::string const_text_value_;

  CoeffList numerator_list_;
  CoeffList denominator_list_;
  std::string generic_math_;

};

}  // namespace mdf


