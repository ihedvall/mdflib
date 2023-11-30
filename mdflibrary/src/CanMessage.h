/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/canmessage.h>

namespace MdfLibrary {

public enum class CanErrorType : uint8_t {
  UNKNOWN_ERROR = 0, ///< Unspecified error.
  BIT_ERROR = 1,     ///< CAN bit error.
  FORM_ERROR = 2,    ///< CAN format error.
  BIT_STUFFING_ERROR = 3, ///< Bit stuffing error.
  CRC_ERROR = 4, ///< Checksum error.
  ACK_ERROR = 5 ///< Acknowledgement error.
};

enum class MessageType : int {
  CAN_DataFrame, ///< Normal CAN message
  CAN_RemoteFrame, ///< Remote frame message.
  CAN_ErrorFrame, ///< Error message.
  CAN_OverloadFrame, ///< Overload frame message.
};

public ref class CanMessage {
public:
  CanMessage();
  virtual ~CanMessage();

  property uint32_t MessageId { void set(uint32_t msgId); uint32_t get();}
  property uint32_t CanId { uint32_t get();}
  property bool ExtendedId { void set(bool extendedId); bool get();}
  property uint8_t Dlc { void set(uint8_t dlc); uint8_t get();}
  property uint32_t DataLength { void set(uint32_t dataLength); uint32_t get();}
  property array<uint8_t>^ DataBytes {
    void set(array<uint8_t>^ dataList);
    array<uint8_t>^ get();
  }
  property uint64_t DataIndex { void set(uint64_t index); uint64_t get();}
  property bool Dir { void set(bool transmit); bool get();}
  property bool Srr { void set(bool srr); bool get();}
  property bool Edl { void set(bool edl); bool get();}
  property bool Brs { void set(bool brs); bool get();}
  property bool Esi { void set(bool esi); bool get();}
  property bool Rtr { void set(bool rtr); bool get();}
  property bool WakeUp { void set(bool wakeUp); bool get();}
  property bool SingleWire { void set(bool singleWire); bool get();}
  property uint8_t BusChannel { void set(uint8_t channel); uint8_t get();}
  property uint8_t BitPosition { void set(uint8_t position); uint8_t get();}
  property CanErrorType ErrorType {
    void set(CanErrorType type);
    CanErrorType get();
  }

  void Reset();
protected:
  !CanMessage();
internal:
  mdf::CanMessage* msg_ = nullptr;
  
};

} 
