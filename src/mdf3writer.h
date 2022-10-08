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
  virtual ~Mdf3Writer();   ///< Destructor that close any open file and
                           ///< destructs.

  IChannel* CreateChannel(IChannelGroup* parent) override;
  IChannelConversion* CreateChannelConversion(IChannel* parent) override;

 protected:
  void CreateMdfFile() override;
  void SetLastPosition(std::FILE* file) override;
};

}  // namespace mdf::detail
