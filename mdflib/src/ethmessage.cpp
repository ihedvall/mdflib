/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/ethmessage.h"
#include "mdf/mdfwriter.h"

#include "littlebuffer.h"
namespace {
  constexpr uint8_t kChannelMask = 0x0F;
  constexpr uint8_t kErrorMask = 0x70;
  constexpr uint8_t kDirMask = 0x80;
}

namespace mdf {
EthMessage::EthMessage(const MdfWriter& writer)
: writer_(writer) {

}

void EthMessage::BusChannel(uint8_t channel) {
  bus_channel_ &= ~kChannelMask;
  bus_channel_ |= channel & kChannelMask;
}

uint8_t EthMessage::BusChannel() const {
  return bus_channel_ & kChannelMask;
}

void EthMessage::Dir(bool transmit) {
  if (transmit) {
    bus_channel_ |= kDirMask;
  } else {
    bus_channel_ &= ~kDirMask;
  }
}

bool EthMessage::Dir() const {
  return (bus_channel_ & kDirMask) != 0;
}

void EthMessage::ErrorType(EthErrorType type) {
  auto temp = static_cast<uint8_t>(type);
  temp <<= 4;
  temp &= kErrorMask;
  bus_channel_ &= ~kErrorMask;
  bus_channel_ |= temp;
}

EthErrorType EthMessage::ErrorType() const {
  uint8_t temp = bus_channel_ & kErrorMask;
  temp >>= 4;
  return static_cast<EthErrorType>(temp);
}

void EthMessage::DataBytes(const std::vector<uint8_t>& data) {
  data_bytes_ = data;
  data_length_ = static_cast<uint16_t>(data_bytes_.size());
}

void EthMessage::Reset() {
  bus_channel_ = 0;
  source_ = {};
  destination_ = {};
  eth_type_ = 0x0800; ///< IPv4 as default
  received_data_byte_count_ = 0;
  data_length_ = 0;
  data_bytes_.clear();
  crc_ = 0;
  expected_crc_ = 0;
  padding_byte_count_ = 0;

}

void EthMessage::ToRaw(EthMessageType msg_type, SampleRecord& sample) const {
  switch (msg_type) {
    case EthMessageType::ETH_Frame:
      MakeDataFrame(sample);
      break;

    case EthMessageType::ETH_ChecksumError:
      MakeChecksumError(sample);
      break;

    case EthMessageType::ETH_LengthError:
      MakeLengthError(sample);
      break;

    case EthMessageType::ETH_ReceiveError:
      MakeReceiveError(sample);
      break;

    default:
      break;
  }

}

void EthMessage::MakeDataFrame(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 8 + 27 : 8 + 33;

  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 8 + 1);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 8 + 7);

  LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 8 + 13);

  LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 8 + 15);

  LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 8 + 17);

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  uint64_t data_index = 0;
  LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 8 + 19);
  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;

  if (mandatory) {
    return;
  }

  LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 8 + 27);

  LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 8 + 31);
}

void EthMessage::MakeChecksumError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 8 + 25 : 8 + 37;

  if (record.size() != record_size) {
    record.resize(record_size);
  }

  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 8 + 1);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 8 + 7);

  LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 8 + 13);

  LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 8 + 15);

  LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 8 + 17);

  LittleBuffer expected_crc(expected_crc_);
  std::copy(expected_crc.cbegin(), expected_crc.cend(), record.begin() + 8 + 21);

  if (mandatory) {
    return;
  }
  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  uint64_t data_index = 0;
  LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 8 + 25);
  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;

  LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 8 + 33);

  LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 8 + 35);
}

void EthMessage::MakeLengthError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 8 + 17 : 8 + 33;

  if (record.size() != record_size) {
    record.resize(record_size);
  }

  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 8 + 1);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 8 + 7);

  LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 8 + 13);

  LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 8 + 15);

  if (mandatory) {
    return;
  }

  LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 8 + 17);

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  uint64_t data_index = 0;
  LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 8 + 19);
  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;

  LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 8 + 27);

  LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 8 + 31);
}

void EthMessage::MakeReceiveError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 8 + 17 : 8 + 33;

  if (record.size() != record_size) {
    record.resize(record_size);
  }

  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 8 + 1);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 8 + 7);

  LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 8 + 13);

  LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 8 + 15);

  if (mandatory) {
    return;
  }

  LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 8 + 17);

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  uint64_t data_index = 0;
  LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 8 + 19);
  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;

  LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 8 + 27);

  LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 8 + 31);
}

}  // namespace mdf