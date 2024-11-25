/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "mdf/mdfwriter.h"

namespace mdf::detail {

class Mdf3Writer : public MdfWriter {
 public:
  Mdf3Writer() = default;  ///< Constructor that creates the ID and HD block.
  ~Mdf3Writer() override;   ///< Destructor that close any open file and
                           ///< destructs.

  IChannelConversion* CreateChannelConversion(IChannel* parent) override;

 protected:
  void CreateMdfFile() override;
  bool PrepareForWriting() override;
  void SetLastPosition(std::streambuf& buffer) override;
};

}  // namespace mdf::detail
