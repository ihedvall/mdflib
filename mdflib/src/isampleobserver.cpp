/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/isampleobserver.h"

namespace mdf {

ISampleObserver::ISampleObserver(const IDataGroup &data_group)
: data_group_(data_group) {
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

void ISampleObserver::OnSample(uint64_t sample, uint64_t record_id,
                               const std::vector<uint8_t> &record) {

}



}