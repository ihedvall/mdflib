/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/ichannelobserver.h"

namespace mdf {

IChannelObserver::IChannelObserver(const IChannel &channel) :
    channel_(channel) {
}

std::string IChannelObserver::Name() const {
  return channel_.Name();
}

std::string IChannelObserver::Unit() const {
  if (channel_.IsUnitValid()) {
    return channel_.Unit();
  }
  const auto* conversion = channel_.ChannelConversion();
  if (conversion != nullptr && conversion->IsUnitValid()) {
    return conversion->Unit();
  }
  return {};
}

const IChannel &IChannelObserver::Channel() const {
  return channel_;
}

bool IChannelObserver::IsMaster() const {
  return channel_.Type() == ChannelType::VirtualMaster  || channel_.Type() == ChannelType::Master;
}

}

