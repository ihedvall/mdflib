/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/isampleobserver.h"

namespace mdf {

ISampleObserver::ISampleObserver(const IDataGroup &data_group)
: data_group_(data_group) {
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
  if (!attached_) {
    attached_ = true;
    data_group_.AttachSampleObserver(this);
  }
}

void ISampleObserver::DetachObserver() {
  if (attached_) {
    data_group_.DetachSampleObserver(this);
    attached_ = false;
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



}