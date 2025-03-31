/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/fhcomment.h"

#include <utility>

namespace mdf {
FhComment::FhComment()
: MdComment("FH") {

}

void FhComment::ToolId(MdString tool_id) {
  tool_id_ = std::move(tool_id);
}

const MdString& FhComment::ToolId() const {
  return tool_id_;
}

void FhComment::ToolVendor(MdString tool_vendor) {
  tool_vendor_ = std::move(tool_vendor);
}

const MdString& FhComment::ToolVendor() const {
  return tool_vendor_;
}

void FhComment::ToolVersion(MdString tool_version) {
  tool_version_ = std::move(tool_version);
}

const MdString& FhComment::ToolVersion() const {
  return tool_version_;
}

void FhComment::UserName(MdString user_name) {
  user_name_ = std::move(user_name);
}

const MdString& FhComment::UserName() const {
  return user_name_;
}

}  // namespace mdf