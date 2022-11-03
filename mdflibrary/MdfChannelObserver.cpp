/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <string>
#include <msclr/marshal_cppstd.h>
#include <mdf/ichannel.h>

#include "MdfChannelObserver.h"

using namespace msclr::interop;
using namespace System;

namespace MdfLibrary {

MdfChannelObserver::~MdfChannelObserver() {
  this->!MdfChannelObserver();
}


size_t MdfChannelObserver::NofSamples::get() {
  return observer_ != nullptr ? observer_->NofSamples() : 0;
}

String^ MdfChannelObserver::Name::get() {
  const auto temp = observer_ != nullptr ?
    observer_->Name() : std::string();
  return gcnew String(temp.c_str());
}

String^ MdfChannelObserver::Unit::get() {
  const auto temp = observer_ != nullptr ?
    observer_->Unit() : std::string();
  return gcnew String(temp.c_str());
}

MdfChannel^ MdfChannelObserver::Channel::get() {
  const auto* temp = observer_ != nullptr ?
    &observer_->Channel() : nullptr;
  return temp != nullptr ?
    gcnew MdfChannel(const_cast<mdf::IChannel*>(temp)) : nullptr;
}

bool MdfChannelObserver::IsMaster() {
  return observer_ != nullptr ? observer_->IsMaster() : false;
}

bool MdfChannelObserver::GetChannelValueAsUnsigned(size_t sample,
  uint64_t% value) {
  uint64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetChannelValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsSigned(size_t sample,
  int64_t% value) {
  int64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetChannelValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsFloat(size_t sample,
  double% value) {
  double temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetChannelValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsString(size_t sample,
  String^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::string temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = gcnew String(temp.c_str());
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsArray(size_t sample,
  array<Byte>^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::vector<uint8_t> temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = gcnew array<Byte>(static_cast<int>(temp.size()));
  for (size_t index = 0; index < temp.size(); ++index) {
    value[static_cast<int>(index)] = temp[index];
  }
  return valid;
}

bool MdfChannelObserver::GetEngValueAsUnsigned(size_t sample, uint64_t% value) {
  uint64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetEngValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetEngValueAsSigned(size_t sample,
  int64_t% value) {
  int64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetEngValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetEngValueAsFloat(size_t sample,
  double% value) {
  double temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetEngValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetEngValueAsString(size_t sample, String^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::string temp;
  const auto valid = observer_->GetEngValue(sample, temp);
  value = gcnew String(temp.c_str());
  return valid;
}

bool MdfChannelObserver::GetEngValueAsArray(size_t sample,
  array<Byte>^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  // Note that engineering value cannot be byte arrays so I assume
  // that it was the channel value that was requested.
  std::vector<uint8_t> temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = gcnew array<Byte>(static_cast<int>(temp.size()));
  for (size_t index = 0; index < temp.size(); ++index) {
    value[static_cast<int>(index)] = temp[index];
  }
  return valid;
}

void MdfChannelObserver::!MdfChannelObserver() {
  delete observer_;
  observer_ = nullptr;
}

MdfChannelObserver::MdfChannelObserver(mdf::IChannelObserver* observer)
  : observer_(observer) {
  
}
}
