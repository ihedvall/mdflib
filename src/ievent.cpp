/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ievent.h"

namespace mdf {

void IEvent::Description(const std::string &description) {
  auto* metadata = MetaData();
  if (metadata != nullptr) {
    metadata->StringProperty("TX", description);
  }
}

std::string IEvent::Description() const {
  const auto* metadata = MetaData();
  return metadata != nullptr ? metadata->StringProperty("TX") : std::string();
}

void IEvent::PreTrig(double pre_trig) {
  auto* metadata = MetaData();
  if (metadata != nullptr) {
    metadata->FloatProperty("pre_trigger_interval", pre_trig);
  }
}

double IEvent::PreTrig() const {
  const auto* metadata = MetaData();
  return metadata != nullptr ? metadata->FloatProperty("pre_trigger_interval") : 0.0;
}

void IEvent::PostTrig(double post_trig) {
  auto* metadata = MetaData();
  if (metadata != nullptr) {
    metadata->FloatProperty("post_trigger_interval", post_trig);
  }
}

double IEvent::PostTrig() const {
  const auto* metadata = MetaData();
  return metadata != nullptr ? metadata->FloatProperty("post_trigger_interval") : 0.0;
}

} // mdf