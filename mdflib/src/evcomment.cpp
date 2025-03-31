/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/evcomment.h"

namespace mdf {

EvComment::EvComment()
: MdComment("EV") {

}

void EvComment::PreTriggerInterval(MdNumber interval) {
  pre_trigger_interval_ = std::move(interval);
}

const MdNumber& EvComment::PreTriggerInterval() const {
  return pre_trigger_interval_;
}

void EvComment::PostTriggerInterval(MdNumber interval) {
  post_trigger_interval_ = std::move(interval);
}

const MdNumber& EvComment::PostTriggerInterval() const {
  return post_trigger_interval_;
}

void EvComment::Timeout(MdNumber timeout) {
  timeout_ = std::move(timeout);
}

const MdNumber& EvComment::Timeout() const {
  return timeout_;
}

}  // namespace mdf