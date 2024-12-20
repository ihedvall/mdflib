/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "busconfigadapter.h"

namespace mdf {

class LinConfigAdapter: public BusConfigAdapter {
 public:
  LinConfigAdapter() = delete;
  explicit LinConfigAdapter(const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;
 private:
  void CreateFrameChannels(IChannelGroup& group);
  void CreateChecksumErrorChannels(IChannelGroup& group);
  void CreateReceiveChannels(IChannelGroup& group);
  void CreateSyncChannels(IChannelGroup& group);
  void CreateWakeUpChannels(IChannelGroup& group);
  void CreateTransmissionErrorChannels(IChannelGroup& group);
  void CreateSpikeChannels(IChannelGroup& group);
  void CreateLongDominantChannels(IChannelGroup& group);
};

}  // namespace mdf


