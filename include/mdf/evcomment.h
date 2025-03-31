/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"
#include "mdf/mdnumber.h"

namespace mdf {

class EvComment : public MdComment {
 public:
  EvComment();

  void PreTriggerInterval(MdNumber interval);
  [[nodiscard]] const MdNumber& PreTriggerInterval() const;

  void PostTriggerInterval(MdNumber interval);
  [[nodiscard]] const MdNumber& PostTriggerInterval() const;

  void Timeout(MdNumber timeout);
  [[nodiscard]] const MdNumber& Timeout() const;

 private:
  MdNumber pre_trigger_interval_;
  MdNumber post_trigger_interval_;
  MdNumber timeout_;
};

}  // namespace mdf

