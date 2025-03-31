/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <limits>

#include "mdf/hoenumerates.h"
#include "mdf/hointerval.h"

#include "ixmlnode.h"

namespace {

constexpr std::string_view kIntervalType = "INTERVAL-TYPE";

}

namespace mdf {

bool HoInterval::IsActive() const {
  return limit_.has_value();
}

void HoInterval::Limit(double limit) {
  limit_ = std::make_optional<double>(limit);
}

double HoInterval::Limit() const {
  return limit_.has_value() ? limit_.value() :
                            std::numeric_limits<double>::quiet_NaN();
}

void HoInterval::Type(HoIntervalType type) {
  type_ = type;
}

HoIntervalType HoInterval::Type() const {
  return type_;
}

void HoInterval::ToXml(IXmlNode& root_node,
                       const std::string_view& tag_name) const {
  if (IsActive()) {
    auto& node = root_node.AddNode(std::string(tag_name));
    node.Value(Limit());
    const HoIntervalType type = Type();
    if (type != HoIntervalType::Closed) {
      node.SetAttribute(std::string(kIntervalType),
                        std::string(HoIntervalTypeToString(type)));
    }
  }

}

}  // namespace mdf