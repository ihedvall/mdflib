/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "mdf/samplerecord.h"

namespace mdf {

class MdfWriter;

enum class EthMessageType : int {
  ETH_Frame,
  ETH_ChecksumError,
  ETH_LengthError,
  ETH_ReceiveError
};

enum class EthErrorType : int {
  Unknown = 0,
  CollisionError = 1
};

class EthMessage {
 public:
  EthMessage() = delete;
  explicit EthMessage(const MdfWriter& writer);

  void BusChannel(uint8_t channel);
  [[nodiscard]] uint8_t BusChannel() const;

  void Dir(bool transmit);
  [[nodiscard]] bool Dir() const;

  void ErrorType(EthErrorType type);
  [[nodiscard]] EthErrorType ErrorType() const;

  void Source(const uint8_t source[6]) {
    std::copy( source , source + source_.size(), source_.data() );
  }
  void Source(const std::array<uint8_t,6>& source) { source_ = source; }
  [[nodiscard]] const std::array<uint8_t, 6>& Source() const { return source_;}

  void Destination(const uint8_t destination[6]) {
    std::copy( destination , destination + destination_.size(), destination_.data() );
  }
  void Destination(const std::array<uint8_t,6>& destination) {
    destination_ = destination;
  }
  [[nodiscard]] const std::array<uint8_t, 6>& Destination() const {
    return destination_;
  }

  void EthType(uint16_t type) { eth_type_ = type; }
  [[nodiscard]] uint16_t EthType() const { return eth_type_; }

  void ReceivedDataByteCount(uint16_t nof_bytes) {
    received_data_byte_count_ = nof_bytes;
  }
  [[nodiscard]] uint16_t ReceivedDataByteCount() const {
    return received_data_byte_count_;
  }

  void DataLength(uint16_t nof_bytes) {
    data_length_ = nof_bytes;
  }

  [[nodiscard]] uint16_t DataLength() const {
    return data_length_;
  }

  void DataBytes(const std::vector<uint8_t>& data);

  [[nodiscard]] const std::vector<uint8_t>& DataBytes() const {
    return data_bytes_;
  }

  void Crc(uint32_t crc) {
    crc_ = crc;
  }

  [[nodiscard]] uint32_t Crc() const {
    return crc_;
  }

  void ExpectedCrc(uint32_t crc) {
    expected_crc_ = crc;
  }

  [[nodiscard]] uint32_t ExpectedCrc() const {
    return expected_crc_;
  }
  void PaddingByteCount(uint16_t nof_bytes) {
    padding_byte_count_ = nof_bytes;
  }

  [[nodiscard]] uint16_t PaddingByteCount() const {
    return padding_byte_count_;
  }

  void Reset();

  void ToRaw(EthMessageType msg_type, SampleRecord& sample) const;

 private:
  uint8_t bus_channel_ = 0; ///< Dir bit 7, Error Type bit 4-6, Bus channel bit 0-3
  std::array<uint8_t,6> source_ = {};
  std::array<uint8_t,6> destination_ = {};
  uint16_t eth_type_ = 0x0800; ///< IPv4 as default
  uint16_t received_data_byte_count_ = 0;
  uint16_t data_length_ = 0;
  std::vector<uint8_t> data_bytes_;
  uint32_t crc_ = 0;
  uint32_t expected_crc_ = 0;
  uint16_t padding_byte_count_ = 0;

  const MdfWriter& writer_;

  void MakeDataFrame(SampleRecord& sample) const;
  void MakeChecksumError(SampleRecord& sample) const;
  void MakeLengthError(SampleRecord& sample) const;
  void MakeReceiveError(SampleRecord& sample) const;
};

}  // namespace mdf


