/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>

#include "mdf/mdcomment.h"
#include "mdf/mdstring.h"

namespace mdf {

class FhComment : public MdComment {
 public:
  FhComment();
  explicit FhComment(std::string comment);

  void ToolId(MdString tool_id);
  [[nodiscard]] const MdString& ToolId() const;

  void ToolVendor(MdString tool_vendor);
  [[nodiscard]] const MdString& ToolVendor() const;

  void ToolVersion(MdString tool_version);
  [[nodiscard]] const MdString& ToolVersion() const;

  void UserName(MdString user_name);
  [[nodiscard]] const MdString& UserName() const;

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;
 private:
  MdString tool_id_;
  MdString tool_vendor_;
  MdString tool_version_;
  MdString user_name_;
};

}  // namespace mdf


