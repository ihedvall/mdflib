/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <optional>

#include "mdf/honamedetails.h"
#include "mdf/hoenumerates.h"
#include "mdf/hoscaleconstraint.h"
#include "mdf/hocompuscale.h"

namespace mdf {


class HoCompuMethod : public HoNameDetails {
 public:
  void Category(HoComputationMethodCategory category);
  [[nodiscard]] HoComputationMethodCategory Category() const;

  void UnitReference(std::string unit_ref);
  [[nodiscard]] const std::string& UnitReference() const;

  void AddPhysicalConstraint(HoScaleConstraint constraint);
  [[nodiscard]] const HoScaleConstraintList& PhysicalConstraints() const;
  [[nodiscard]] HoScaleConstraintList& PhysicalConstraints();

  void AddInternalConstraint(HoScaleConstraint constraint);
  [[nodiscard]] const HoScaleConstraintList& InternalConstraints() const;
  [[nodiscard]] HoScaleConstraintList& InternalConstraints();

 private:
  HoComputationMethodCategory category_ = HoComputationMethodCategory::Identical;
  std::string unit_ref_;
  HoScaleConstraintList physical_constraint_list_;
  HoScaleConstraintList internal_constraint_list_;
  HoCompuScaleList compu_scale_list_;

  std::optional<double> default_float_value_;
  std::string default_text_value_;
};

}  // namespace mdf


