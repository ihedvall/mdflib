/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/canmessage.h"
#include <array>
#include "mdf/ichannel.h"


namespace {
constexpr uint32_t kExtendedBit    = 0x80000000;
constexpr uint32_t k11BitMask      = 0x7FF;
constexpr uint8_t kDirBit         = 0x01;
constexpr uint8_t kSrrBit         = 0x02;
constexpr uint8_t kEdlBit         = 0x04;
constexpr uint8_t kBrsBit         = 0x08;
constexpr uint8_t kEsiBit         = 0x10;
constexpr uint8_t kWakeUpBit      = 0x20;
constexpr uint8_t kSingleWireBit  = 0x40;
constexpr uint8_t kRtrBit         = 0x80;

constexpr std::array<size_t,16> kDataLengthCode =
    {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
}

namespace mdf {

void CanMessage::MessageId(uint32_t msg_id) {
  message_id_ = msg_id;
  if (msg_id > k11BitMask) {
    // more than 11 bit means extended ID
    message_id_ |= kExtendedBit;
  }
}

uint32_t CanMessage::MessageId() const { return message_id_; }

void CanMessage::ExtendedId(bool extended) {
  if (extended) {
    message_id_ |= kExtendedBit;
  } else {
    message_id_ &= ~kExtendedBit;
  }
}

bool CanMessage::ExtendedId() const {
  return (message_id_ & kExtendedBit) != 0;
}

void CanMessage::Dlc(uint8_t dlc) {
  dlc_ = 0x0F & dlc;
  const auto data_size = kDataLengthCode[dlc_ & 0x0F];
  if (data_bytes_.size() != data_size) {
    data_bytes_.resize(data_size);
  }
}

uint8_t CanMessage::Dlc() const { return dlc_ & 0x0F; }

void CanMessage::DataLength(size_t data_length) {
  if (data_length > 64) {
    data_length = 8;
  }
  uint8_t dlc = 0;
  for (const auto data_size : kDataLengthCode) {
    if (data_length <= data_size) {
      break;
    }
    ++dlc;
  }
  Dlc(dlc);
}

size_t CanMessage::DataLength() const {
  return kDataLengthCode[dlc_ & 0x0F];
}

void CanMessage::DataBytes(const std::vector<uint8_t>& data) {
  DataLength(data.size()); // This fix the length issue in CAN FD
  size_t index;
  for (index = 0; index < data.size() && index < data_bytes_.size(); ++index) {
    data_bytes_[index] = data[index];
  }
  for (; index < data_bytes_.size(); ++index) {
    data_bytes_[index] = 0xFF;
  }
}

const std::vector<uint8_t>& CanMessage::DataBytes() const {
  return data_bytes_;
}

void CanMessage::Dir(bool transmit) {
  if (transmit) {
    flags_ |= kDirBit;
  } else {
    flags_ &= ~kDirBit;
  }
}

bool CanMessage::Dir() const {
  return (flags_ & kDirBit) != 0;
}

void CanMessage::Srr(bool srr) {
  if (srr) {
    flags_ |= kSrrBit;
  } else {
    flags_ &= ~kSrrBit;
  }
}

bool CanMessage::Srr() const {
  return (flags_ & kSrrBit) != 0;
}

void CanMessage::Edl(bool edl) {
  if (edl) {
    flags_ |= kEdlBit;
  } else {
    flags_ &= ~kEdlBit;
  }
}

bool CanMessage::Edl() const {
  return (flags_ & kEdlBit) != 0;
}

void CanMessage::Brs(bool brs) {
  if (brs) {
    flags_ |= kBrsBit;
  } else {
    flags_ &= ~kBrsBit;
  }
}

bool CanMessage::Brs() const {
  return (flags_ & kBrsBit) != 0;
}

void CanMessage::Esi(bool esi) {
  if (esi) {
    flags_ |= kEsiBit;
  } else {
    flags_ &= ~kEsiBit;
  }
}

bool CanMessage::Esi() const {
  return (flags_ & kEsiBit) != 0;
}

void CanMessage::Rtr(bool rtr) {
  if (rtr) {
    flags_ |= kRtrBit;
  } else {
    flags_ &= ~kRtrBit;
  }
}

bool CanMessage::Rtr() const {
  return (flags_ & kRtrBit) != 0;
}

void CanMessage::WakeUp(bool wake_up) {
  if (wake_up) {
    flags_ |= kWakeUpBit;
  } else {
    flags_ &= ~kWakeUpBit;
  }
}

bool CanMessage::WakeUp() const {
  return (flags_ & kWakeUpBit) != 0;
}

void CanMessage::SingleWire(bool single_wire) {
  if (single_wire) {
    flags_ |= kSingleWireBit;
  } else {
    flags_ &= ~kSingleWireBit;
  }
}

bool CanMessage::SingleWire() const {
  return (flags_ & kSingleWireBit) != 0;
}

void CanMessage::DataIndex(uint64_t index) {
  data_index_ = index;
}

uint64_t CanMessage::DataIndex() const {
  return data_index_;
}

void CanMessage::BusChannel(uint8_t channel) {
  dlc_ &= 0x0F;
  dlc_ |= channel << 4;
}

uint8_t CanMessage::BusChannel() const {
  return (dlc_ & 0xF0) >> 4;
}

void CanMessage::BitPosition(uint8_t position) {
  bit_position_ = position;
}

uint8_t CanMessage::BitPosition() const {
  return bit_position_;
}

void CanMessage::ErrorType(mdf::CanErrorType error_type) {
  error_type_ = static_cast<uint8_t>(error_type);
}

CanErrorType CanMessage::ErrorType() const {
  return static_cast<CanErrorType>(error_type_);
}

size_t CanMessage::DlcToLength(uint8_t dlc) {
  return dlc < kDataLengthCode.size() ? kDataLengthCode[dlc] : 0;
}

}  // namespace mdf