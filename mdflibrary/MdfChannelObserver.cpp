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

void MdfChannelObserver::!MdfChannelObserver() {
  delete observer_;
  observer_ = nullptr;
}

MdfChannelObserver::MdfChannelObserver(mdf::IChannelObserver* observer)
  : observer_(observer) {
  
}

template <>
bool MdfChannelObserver::GetChannelValue(size_t sample,
  String^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::string temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = gcnew String(temp.c_str());
  return valid;
}

template <>
bool MdfChannelObserver::GetChannelValue(size_t sample,
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

template <>
bool MdfChannelObserver::GetEngValue(size_t sample,
  String^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::string temp;
  const auto valid = observer_->GetEngValue(sample, temp);
  value = gcnew String(temp.c_str());
  return valid;
}

template <>
bool MdfChannelObserver::GetEngValue(size_t sample,
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

}
