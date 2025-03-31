/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include "mdf/hocompumethod.h"

#include <utility>

namespace mdf {

void HoCompuMethod::Category(HoComputationMethodCategory category) {
  category_ = category;
}

HoComputationMethodCategory HoCompuMethod::Category() const {
  return category_;
}

void HoCompuMethod::UnitReference(std::string unit_ref) {
  unit_ref_ = std::move(unit_ref);
}

const std::string& HoCompuMethod::UnitReference() const {
  return unit_ref_;
}

void HoCompuMethod::AddPhysicalConstraint(HoScaleConstraint constraint) {
  physical_constraint_list_.emplace_back(constraint);
}

const HoScaleConstraintList& HoCompuMethod::PhysicalConstraints() const {
  return physical_constraint_list_;
}

HoScaleConstraintList& HoCompuMethod::PhysicalConstraints() {
  return physical_constraint_list_;
}

void HoCompuMethod::AddInternalConstraint(HoScaleConstraint constraint) {
  internal_constraint_list_.emplace_back(constraint);
}

const HoScaleConstraintList& HoCompuMethod::InternalConstraints() const {
  return internal_constraint_list_;
}

HoScaleConstraintList& HoCompuMethod::InternalConstraints() {
  return internal_constraint_list_;
}

}  // namespace mdf