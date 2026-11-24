/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/isampleobserver.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"

namespace mdf {

ISampleObserver::ISampleObserver(const IDataGroup &data_group)
: data_group_(&data_group) {
  ISampleObserver::AttachObserver();
  // Subscribe on all channel groups
  const auto cg_list = data_group.ChannelGroups();
  record_id_list_.clear();
  for (const auto* channel_group : cg_list) {
    if ( channel_group != nullptr) {
      record_id_list_.insert(channel_group->RecordId());
    }
  }
}

ISampleObserver::~ISampleObserver() {
  ISampleObserver::DetachObserver();
}

void ISampleObserver::AttachObserver() {
  if (data_group_ != nullptr) {
     data_group_->AttachSampleObserver(this);
  }
}

void ISampleObserver::DetachObserver() {
  if (data_group_ != nullptr) {
    data_group_->DetachSampleObserver(this);
    data_group_ = nullptr;
  }
}

bool ISampleObserver::OnSample(uint64_t sample, uint64_t record_id,
                               const std::vector<uint8_t> &record) {
  bool continue_reading = true;
  if (DoOnSample) {
    continue_reading = DoOnSample(sample, record_id, record);
  }
  return continue_reading;
}


void ISampleObserver::FindVlsdRecord(const IChannelGroup& channel_group) {
  const auto channel_list = channel_group.Channels();
  const IChannel* first_vlsd = nullptr;
  for (const auto* channel : channel_list) {
    if (channel == nullptr ||
         channel->Type() != ChannelType::VariableLength)  {
      continue;
    }
    if (first_vlsd == nullptr) {
      first_vlsd = channel;
    }
    // Text and other VLSD types are passed on as raw bytes, but only for the
    // first VLSD channel, the one that Cg4Block::FindSdChannel() writes.
    if (channel->DataType() != ChannelDataType::ByteArray &&
        channel != first_vlsd) {
      continue;
    }
    // The signal data index point to either a CG VLSD record or a SD(HL/DL/DZ) block
    // MDF 3 doesn't support VLSD.
    IBlock* sd_block = channel->GetVlsdBlock();
    if (sd_block == nullptr) {
      continue;
    }
    if (sd_block->BlockType() == "CG") {
      // The VLSD link is pointing on a CG block
      if (auto* cg_block = dynamic_cast<IChannelGroup*>(sd_block);
          cg_block != nullptr) {
        record_id_list_.insert(cg_block->RecordId());
        cg_vlsd_list_.emplace(channel, cg_block);
      }
    } else {
      // The VLSD link is pointing on a SD/HL/DL or DZ block
      sd_vlsd_list_.emplace_back(channel);
    }
  }
}


}