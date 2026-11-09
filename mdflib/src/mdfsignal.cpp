//
// Created by ihedv on 2026-11-08.
//

#include "mdfsignal.h"

namespace mdf {
MdfSignal::MdfSignal(MdfReader* reader, IDataGroup* data_group,
        const IChannelGroup* channel_group, const IChannel* channel)
: ISignal(),
  reader_(reader),
  data_group_(data_group),
  channel_group_(channel_group),
  channel_(channel) {
  if (channel_ != nullptr) {
    name_ = channel->DisplayName();
    if (name_.empty()) {
      name_ = channel->Name();
    }
  }
}

std::string MdfSignal::Description() const {
  return channel_ != nullptr ? channel_->Description() : std::string();
}

std::string MdfSignal::Unit() const {
  return channel_ != nullptr ? channel_->Unit() : std::string();
}

size_t MdfSignal::NofSamples() const {
   return channel_group_ != nullptr ? channel_group_->NofSamples() : 0;
}

std::string MdfSignal::Source() const {
  std::ostringstream temp;
  if (data_group_ != nullptr) {
    temp << data_group_->Name() << "/";
  }
  if (channel_group_ != nullptr) {
    temp << channel_group_->Name() << "/";
  }
  if (channel_ != nullptr) {
    temp << channel_->Name();
  }
  return temp.str();
}

MdfReader* MdfSignal::Reader() const {
  return reader_;
}

int64_t MdfSignal::BlockIndex() const {
  return channel_ != nullptr ? channel_->Index() : -1;
}

const IChannelObserver* MdfSignal::SignalData() const {
  return nullptr;
}

const IChannelObserver* MdfSignal::MasterData() const {
  return nullptr;
}

} // mdf