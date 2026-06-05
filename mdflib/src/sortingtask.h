/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */


#pragma once
#include "mdf/mdftask.h"

namespace mdf {

class SortingTask : public MdfTask {
public:
  SortingTask() = default;
  ~SortingTask() override = default;

  void Run() override;

private:

  void SortFile();
};

}  // namespace mdf

