/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */


#pragma once
#include "mdf/mdftask.h"

namespace mdf {

class ValidatingTask : public MdfTask {
 public:
  ValidatingTask() = default;
  ~ValidatingTask() override = default;
  void Run() override;
};

}  // namespace mdf


