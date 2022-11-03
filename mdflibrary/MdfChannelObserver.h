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
  
  bool GetChannelValueAsUnsigned(size_t sample, uint64_t% value);
  bool GetChannelValueAsSigned(size_t sample, int64_t% value);
  bool GetChannelValueAsFloat(size_t sample, double% value);  
  bool GetChannelValueAsString(size_t sample, String^% value);    
  bool GetChannelValueAsArray(size_t sample,  array<Byte>^% value);

   bool GetEngValueAsUnsigned(size_t sample, uint64_t% value);
   bool GetEngValueAsSigned(size_t sample, int64_t% value);
   bool GetEngValueAsFloat(size_t sample, double% value);  
   bool GetEngValueAsString(size_t sample, String^% value);    
   bool GetEngValueAsArray(size_t sample,  array<Byte>^% value);
  
protected:
  !MdfChannelObserver();
private:
  MdfChannelObserver() {}
  MdfChannelObserver(MdfChannelObserver^ &observer) {}
internal:  
  mdf::IChannelObserver *observer_ = nullptr;
  MdfChannelObserver(mdf::IChannelObserver *observer);
};

}
