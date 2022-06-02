/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once


#include "mdf/mdfwriter.h"


namespace mdf::detail {

class Mdf4Writer : public MdfWriter {
 public:
  Mdf4Writer() = default;
  virtual ~Mdf4Writer();


  IChannel* CreateChannel(IChannelGroup* parent) override;
  IChannelConversion* CreateChannelConversion(IChannel* parent) override;
 protected:
  void CreateMdfFile() override;
  void SetLastPosition(std::FILE* file) override;
 private:

};

} // mdf
