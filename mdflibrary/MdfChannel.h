/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/ichannel.h>
#include "MdfChannelConversion.h"

using namespace System;
namespace MdfLibrary {

public enum class ChannelType : uint8_t {
  FixedLength = 0,
  VariableLength = 1,
  Master = 2,
  VirtualMaster = 3,
  Sync = 4,
  MaxLength = 5,
  VirtualData = 6
};

public enum class ChannelSyncType : uint8_t {
  None = 0,
  Time = 1,
  Angle = 2,
  Distance = 3,
  Index = 4
};

public enum class ChannelDataType : uint8_t {
  UnsignedIntegerLe = 0,
  UnsignedIntegerBe = 1,
  SignedIntegerLe = 2,
  SignedIntegerBe = 3,
  FloatLe = 4,
  FloatBe = 5,
  StringAscii = 6,
  StringUTF8 = 7,
  StringUTF16Le = 8,
  StringUTF16Be = 9,
  ByteArray = 10,
  MimeSample = 11,
  MimeStream = 12,
  CanOpenDate = 13,
  CanOpenTime = 14,
  ComplexLe = 15,
  ComplexBE = 16
};

public ref class MdfChannel {
public:
  literal uint32_t AllValuesInvalid = 0x0001;
  literal uint32_t InvalidValid = 0x0002;
  literal uint32_t PrecisionValid = 0x0004;
  literal uint32_t RangeValid = 0x0008;
  literal uint32_t LimitValid = 0x0010;
  literal uint32_t ExtendedLimitValid = 0x0020;
  literal uint32_t Discrete = 0x0040;
  literal uint32_t Calibration = 0x0080;
  literal uint32_t Calculated = 0x0100;
  literal uint32_t Virtual = 0x0200;
  literal uint32_t BusEvent = 0x0400;
  literal uint32_t StrictlyMonotonous = 0x0800;
  literal uint32_t DefaultX = 0x1000;
  literal uint32_t EventSignal = 0x2000;
  literal uint32_t VlsdDataStream = 0x4000;
  
  property int64_t Index { int64_t get(); }
  property String^ Name { String^ get(); void set(String^ name); }
  property String^ DisplayName { String^ get(); void set(String^ name); }
  property String^ Description { String^ get(); void set(String^ desc); }
  
  property bool UnitUsed { bool get(); }
  property String^ Unit { String^ get(); void set(String^ unit); }

  property ChannelType Type { ChannelType get(); void set(ChannelType type); }
  property ChannelSyncType Sync {
    ChannelSyncType get();
    void set(ChannelSyncType type);
  }
  property ChannelDataType DataType {
    ChannelDataType get();
    void set(ChannelDataType type);
  }

  property size_t DataBytes { size_t get(); void set(size_t bytes); };  

  property bool PrecisionUsed { bool get(); }
  property Byte Precision { Byte get(); };
  
  property bool RangeUsed { bool get(); }
  property Tuple<double,double>^ Range {
    Tuple<double,double>^ get();
    void set(Tuple<double,double>^);
  };
  
  property bool LimitUsed { bool get(); }
  property Tuple<double,double>^ Limit {
    Tuple<double,double>^ get();
    void set(Tuple<double,double>^);
  };

  property bool ExtLimitUsed { bool get(); }
  property Tuple<double,double>^ ExtLimit {
    Tuple<double,double>^ get();
    void set(Tuple<double,double>^);
  };

  property double SamplingRate { double get(); void set(double rate); }

  property MdfChannelConversion^ ChannelConversion {
      MdfChannelConversion^ get(); 
  }

  MdfChannelConversion^ CreateMdfChannelConversion();

 private:
  MdfChannel() {}
internal:
  mdf::IChannel* channel_ = nullptr;
  MdfChannel(mdf::IChannel* channel);


};
}