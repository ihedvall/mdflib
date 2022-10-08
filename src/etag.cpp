/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/etag.h"

namespace mdf {

template <>
void ETag::Value(const bool& value) {
  value_ = value ? "true" : "false";
}

template <>
[[nodiscard]] bool ETag::Value() const {
  if (value_.empty()) {
    return false;
  }
  return value_[0] == 'T' || value_[0] == 't' || value_[0] == 'Y' ||
         value_[0] == 'y' || value_[0] == '1';
}

void ETag::AddTag(const ETag& tag) { tree_list_.push_back(tag); }

}  // namespace mdf
