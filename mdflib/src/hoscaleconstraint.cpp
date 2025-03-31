/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <string_view>

#include "mdf/hoscaleconstraint.h"
#include "mdf/hoenumerates.h"

#include "ixmlnode.h"

namespace {
constexpr std::string_view kValidity = "VALIDITY";

constexpr std::string_view kLowerLimit = "ho:LOWER-LIMIT";
constexpr std::string_view kUpperLimit = "ho:UPPER-LIMIT";

constexpr std::string_view kScaleConstraint = "ho:SCALE-CONSTR";

}

namespace mdf {

bool HoScaleConstraint::IsActive() const {
  return lower_limit_.IsActive() || upper_limit_.IsActive();
}

void HoScaleConstraint::LowerLimit(HoInterval limit) {
  lower_limit_ = limit;
}

const HoInterval& HoScaleConstraint::LowerLimit() const {
  return lower_limit_;
}

void HoScaleConstraint::UpperLimit(HoInterval limit) {
  upper_limit_ = limit;
}

const HoInterval& HoScaleConstraint::UpperLimit() const {
  return upper_limit_;
}

void HoScaleConstraint::Validity(HoValidity validity) {
  validity_ = validity;
}

HoValidity HoScaleConstraint::Validity() const {
  return validity_;
}

void HoScaleConstraint::ToXML(IXmlNode& root_node) const {
  // Assume that the ho namespace is set in the MD comment root
  if (!IsActive()) {
    return;
  }
  auto& scale_node = root_node.AddNode(std::string(kScaleConstraint));
  lower_limit_.ToXml(scale_node, kLowerLimit);
  upper_limit_.ToXml(scale_node, kUpperLimit);

  if (validity_ != HoValidity::Valid) {
    scale_node.SetAttribute(std::string(kValidity),
                         std::string(HoValidityToString(validity_)));
  }
}

}  // namespace mdf