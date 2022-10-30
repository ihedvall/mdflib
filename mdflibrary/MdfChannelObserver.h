/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/ichannelobserver.h"
#include "MdfChannel.h"

using namespace System;

namespace MdfLibrary {

public ref class MdfChannelObserver {
public:
  virtual ~MdfChannelObserver();

  property size_t NofSamples { size_t get(); }
  property String^ Name { String^ get(); }
  property String^ Unit { String^ get(); }
  property MdfChannel^ Channel { MdfChannel^ get(); }

  bool IsMaster();
  
  template <class T>
  bool GetChannelValue(size_t sample, T% value);
  
  template <class T>
  bool GetEngValue(size_t sample, T% value);
  
protected:
  !MdfChannelObserver();
private:
  MdfChannelObserver() {}
  MdfChannelObserver(MdfChannelObserver^ &observer) {}
internal:  
  mdf::IChannelObserver *observer_ = nullptr;
  MdfChannelObserver(mdf::IChannelObserver *observer);
};

template <class T>
inline bool MdfChannelObserver::GetChannelValue(size_t sample, T% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  T temp {};
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = temp;
  return valid;
}

template <>
bool MdfChannelObserver::GetChannelValue(size_t sample, String^% value);

template <>
bool MdfChannelObserver::GetChannelValue(size_t sample, array<Byte>^% value);

template <class T>
inline bool MdfChannelObserver::GetEngValue(size_t sample, T% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  T temp {};
  const auto valid = observer_->GetEngValue(sample, temp);
  value = temp;
  return valid;
}

template <>
bool MdfChannelObserver::GetEngValue(size_t sample, String^% value);

template <>
bool MdfChannelObserver::GetEngValue(size_t sample, array<Byte>^% value);
}
