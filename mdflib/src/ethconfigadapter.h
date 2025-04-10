/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "busconfigadapter.h"

namespace mdf {

class EthConfigAdapter : public BusConfigAdapter {
 public:
  EthConfigAdapter() = delete;
  explicit EthConfigAdapter(const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;
 private:
  void CreateFrameChannels(IChannelGroup& group);
  void CreateChecksumErrorChannels(IChannelGroup& group);
  void CreateLengthErrorChannels(IChannelGroup& group);
  void CreateReceiveErrorChannels(IChannelGroup& group);
};

}  // namespace mdf

