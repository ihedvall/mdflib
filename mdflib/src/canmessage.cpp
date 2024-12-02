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
constexpr uint8_t kMask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

void UnsignedToRaw(bool little_endian, size_t start, size_t length,
                   uint64_t value, uint8_t* raw) {
  if (raw == nullptr || length == 0) {
    return;
  }

  uint64_t mask = 1ULL << (length - 1);
  auto bit = little_endian ? static_cast<int>(start + length - 1)
                           : static_cast<int>(start);
  auto byte = bit / 8;
  bit %= 8;

  for (size_t index = 0; index < length; ++index) {
    if ((value & mask) != 0) {
      raw[byte] |= kMask[bit];
    } else {
      raw[byte] &= ~kMask[bit];
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }
}

} // end namespace

namespace mdf {

void CanMessage::MessageId(uint32_t msg_id) {
  message_id_ = msg_id;
  if (msg_id > k11BitMask) {
    // more than 11 bit means extended ID
    message_id_ |= kExtendedBit;
  }
}

uint32_t CanMessage::MessageId() const { return message_id_; }

uint32_t CanMessage::CanId() const { return message_id_ & ~kExtendedBit; }

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
  dlc_ &= 0xF0;
  dlc_ |=  dlc;
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

void CanMessage::ToRaw( MessageType msg_type, SampleRecord& sample,
                       size_t max_data_length, bool save_index ) const {
  size_t record_size = 0;
  if (max_data_length < 8) {
    max_data_length = 8;
  }
  auto& record = sample.record_buffer;
  switch (msg_type) {
    case MessageType::CAN_DataFrame:
      record_size = 8 + 4 + 1 + 1;
      record_size += save_index ? 8 : max_data_length;
      if (record.size() != record_size) {
        record.resize(record_size);
      }
      UnsignedToRaw(true, 8*8, 32, message_id_, record.data());
      record[8+4] =  dlc_;
      record[8+5] = flags_;
      if (save_index) {
        // The data index have in reality not been updated at this point but
        // it will be updated when the sample buffer is written to the disc.
        // We need to save the data bytes to a temp buffer (VLSD data).
        sample.vlsd_data = true;
        sample.vlsd_buffer = data_bytes_;
        UnsignedToRaw(true, (8+6)*8, 64, data_index_, record.data());
      } else {
        for (size_t index = 0; index < max_data_length; ++index) {
          record[14 + index] =
              index < data_bytes_.size() ? data_bytes_[index] : 0xFF;
        }
      }
      break;

    case MessageType::CAN_RemoteFrame:
      record_size = 8 + 4 + 1 + 1;
      if (record.size() != record_size) {
        record.resize(record_size);
      }
      UnsignedToRaw(true, 8*8, 32, message_id_, record.data());
      record[8+4] = dlc_;
      record[8+5] = flags_;
      break;

    case MessageType::CAN_ErrorFrame:
      record_size = 8 + 4 + 1 + 1 + 1 + 1;
      record_size += save_index ? 8 : max_data_length;
      if (record.size() < record_size) {
        record.resize(record_size);
      }
      UnsignedToRaw(true, 8*8, 32, message_id_, record.data());
      record[8+4] =  dlc_;
      record[8+5] = flags_;
      record[8+6] = bit_position_;
      record[8+7] = error_type_;
      if (save_index) {
        sample.vlsd_data = true;
        sample.vlsd_buffer = data_bytes_;
        UnsignedToRaw(true, (8+8)*8, 64, data_index_, record.data());
      } else {
        for (size_t index = 0; index < max_data_length; ++index) {
        record[16 + index] = index < data_bytes_.size()
                                 ? data_bytes_[index] : 0xFF;
        }
      }
      break;

    case MessageType::CAN_OverloadFrame:
      record_size = 8 + 1;
      if (record.size() != record_size) {
        record.resize(record_size);
      }
      record[8] = (dlc_ & 0xF0) | (flags_ & 0x01);
      break;

    default:
      break;
  }
}

void CanMessage::Reset() {
  message_id_ = 0;
  dlc_ = 0;
  flags_ = 0;
  data_index_ = 0;
  data_bytes_.clear();
  bit_position_ = 0;
  error_type_ = 0;
}

}  // namespace mdf