/*
* Copyright 2026 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iconfigadapter.h"
#include "mdf/mdfreader.h"

namespace mdf {

class SortingConfigAdapter : public IConfigAdapter {
public:
  SortingConfigAdapter() = delete;
  explicit SortingConfigAdapter(const MdfWriter& writer, MdfReader& reader);
  void CreateConfig(IHeader& header) override;

private:
  MdfReader& reader_;
};

}  // namespace mdf


