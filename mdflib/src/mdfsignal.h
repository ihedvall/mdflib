/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/isignal.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"



namespace mdf {

class MdfSignal : public ISignal {
public:
  MdfSignal() = delete;
  explicit MdfSignal(MdfReader* reader,
                     IDataGroup* data_group,
                     const IChannelGroup* channel_group,
                     const IChannel* channel);
  ~MdfSignal() override = default;
  [[nodiscard]] std::string Description() const override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] size_t NofSamples() const override;
  [[nodiscard]] std::string Source() const override;
  [[nodiscard]] MdfReader* Reader() const override;
  [[nodiscard]] int64_t BlockIndex() const override;
  [[nodiscard]] const IChannelObserver* SignalData() const override;
  [[nodiscard]] const IChannelObserver* MasterData() const override;

 protected:
  MdfReader* reader_ = nullptr;
  IDataGroup* data_group_ = nullptr;
  const IChannelGroup* channel_group_ = nullptr;
  const IChannel* channel_ = nullptr;

};

}  // namespace mdf

