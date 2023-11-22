/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once

#include <cstdint>
#include <vector>
namespace mdf {

enum class CanErrorType : uint8_t {
  UNKNOWN_ERROR = 0,
  BIT_ERROR = 1,
  FORM_ERROR = 2,
  BIT_STUFFING_ERROR = 3,
  CRC_ERROR = 4,
  ACK_ERROR = 5
};

class IChannel;

class CanMessage {
 public:
  void MessageId(uint32_t msg_id);
  [[nodiscard]] uint32_t MessageId() const;

  void ExtendedId(bool extended );
  [[nodiscard]] bool ExtendedId() const;

  void Dlc(uint8_t dlc);
  [[nodiscard]] uint8_t Dlc() const;

  void DataLength(size_t data_length);
  [[nodiscard]] size_t DataLength() const;

  void DataBytes(const std::vector<uint8_t>& data);
  [[nodiscard]] const std::vector<uint8_t>& DataBytes() const;

  void DataIndex(uint64_t index);
  [[nodiscard]] uint64_t DataIndex() const;

  void Dir(bool transmit );
  [[nodiscard]] bool Dir() const;

  void Srr(bool srr );
  [[nodiscard]] bool Srr() const;

  void Edl(bool edl );
  [[nodiscard]] bool Edl() const;

  void Brs(bool brs );
  [[nodiscard]] bool Brs() const;

  void Esi(bool esi );
  [[nodiscard]] bool Esi() const;

  void Rtr(bool rtr );
  [[nodiscard]] bool Rtr() const;

  void WakeUp(bool wake_up );
  [[nodiscard]] bool WakeUp() const;

  void SingleWire(bool single_wire );
  [[nodiscard]] bool SingleWire() const;

  void BusChannel(uint8_t channel);
  [[nodiscard]] uint8_t BusChannel() const;

  void BitPosition(uint8_t position);
  [[nodiscard]] uint8_t BitPosition() const;

  void ErrorType(CanErrorType error_type);
  [[nodiscard]] CanErrorType ErrorType() const;

  static size_t DlcToLength(uint8_t dlc);

 private:
  uint32_t message_id_ = 0;
  uint8_t  dlc_ = 0;
  uint8_t flags_ = 0;
  uint64_t data_index_ = 0;
  std::vector<uint8_t> data_bytes_;
  uint8_t bit_position_ = 0;
  uint8_t error_type_ = 0;

};

}  // namespace mdf
