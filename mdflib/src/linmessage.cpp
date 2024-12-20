/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/linmessage.h"

#include "mdf/mdfwriter.h"

#include "littlebuffer.h"

namespace {
  constexpr uint8_t kChecksumModelMask = 0xC0; ///< Checksum model 2-bit
  constexpr uint8_t kChannelMask = 0x3F; ///< Channel 6-bit

  constexpr uint8_t kLinIdMask = 0x3F; ///< 6 bit mask
  constexpr uint8_t kDirMask = 0x80;   ///< High bit mask
  constexpr uint8_t kReceivedMask = 0x0F; ///< Low 4 bit
  constexpr uint8_t kLengthMask = 0xF0;   ///< High 4 bit
  constexpr uint8_t kExpectedLength = 0x0F;
  constexpr uint8_t kDominantTypeMask = 0x30;
}

namespace mdf {

LinMessage::LinMessage(const MdfWriter& writer)
: writer_(writer) {

}

void LinMessage::BusChannel(uint8_t channel) {
  bus_channel_ &= ~kChannelMask;
  bus_channel_ |= channel & kChannelMask;
}

uint8_t LinMessage::BusChannel() const {
  return bus_channel_ & kChannelMask;
}

void LinMessage::LinId(uint8_t id) {
  lin_id_ &= ~kLinIdMask;
  lin_id_ |= (id & kLinIdMask);
}

uint8_t LinMessage::LinId() const {
  return lin_id_ & kLinIdMask;
}

void LinMessage::Dir(bool transmit) {
  if (transmit) {
    lin_id_ |= kDirMask;
  } else {
    lin_id_ &= ~kDirMask;
  }
}

bool LinMessage::Dir() const {
  return (lin_id_ & kDirMask) != 0;
}

void LinMessage::ReceivedDataByteCount(uint8_t nof_bytes) {
  data_length_ &= ~kReceivedMask;
  data_length_ |= nof_bytes & kReceivedMask;
}

uint8_t LinMessage::ReceivedDataByteCount() const {
  return data_length_ & kReceivedMask;
}

void LinMessage::DataLength(uint8_t nof_bytes) {
  data_length_ &= ~kLengthMask;
  data_length_ |= (nof_bytes << 4) & kLengthMask;
}

uint8_t LinMessage::DataLength() const {
  return (data_length_ & kLengthMask) >> 4;
}

void LinMessage::ChecksumModel(LinChecksumModel model) {
  uint8_t val;
  switch (model) {
    case LinChecksumModel::Classic:
      val = 0;
      break;

    case LinChecksumModel::Enhanced:
      val = 0x01 << 6;
      break;

    default:
      val = kChecksumModelMask; // -1
      break;

  }
  bus_channel_ &= ~kChecksumModelMask;
  bus_channel_ |= val;
}

LinChecksumModel LinMessage::ChecksumModel() const {
  const uint8_t val = (bus_channel_ & kChecksumModelMask) >> 6;
  switch (val) {
    case 0: return LinChecksumModel::Classic;
    case 1: return LinChecksumModel::Enhanced;
    default:
      break;
  }
  return LinChecksumModel::Unknown;
}

void LinMessage::Reset() {
  bus_channel_ = 0xC0;
  lin_id_ = 0;
  data_length_ = 0;
  crc_ = 0;
  sof_ = 0;
  baudrate_ = 0.0;
  response_baudrate_ = 0.0;
  break_length_ = 0;
  delimiter_break_length_ = 0;
  data_bytes_.clear();
  spare_ = 0;
  total_signal_length_ = 0;
}

void LinMessage::ToRaw( LinMessageType msg_type, SampleRecord& sample) const {

  switch (msg_type) {
    case LinMessageType::LIN_Frame:
      MakeDataFrame(sample);
      break;

    case LinMessageType::LIN_WakeUp:
      MakeWakeUp(sample);
      break;

    case LinMessageType::LIN_ChecksumError:
      MakeChecksumError(sample);
      break;

    case LinMessageType::LIN_TransmissionError:
      MakeTransmissionError(sample);
      break;

    case LinMessageType::LIN_SyncError:
      MakeSyncError(sample);
      break;

    case LinMessageType::LIN_ReceiveError:
      MakeReceiveError(sample);
      break;

    case LinMessageType::LIN_Spike:
      MakeSpike(sample);
      break;

    case LinMessageType::LIN_LongDominantSignal:
      MakeLongDominantSignal(sample);
      break;

    default:
      break;
  }
}

void LinMessage::MakeDataFrame(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 19 : 44;

  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  record[8 + 1] = lin_id_;
  record[8 + 2] = data_length_;

  // The MLSD storage is fixed in LIN.
  for (size_t index = 0; index < 8; ++index) {
    record[11 + index] =
        index < data_bytes_.size() ? data_bytes_[index] : 0xFF;
  }
  if (mandatory) {
    return;
  }

  record[19] = crc_;

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(),
            record.begin() + 20);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(),
            record.begin() + 28);

  LittleBuffer response_baudrate(response_baudrate_);
  std::copy(response_baudrate.cbegin(), response_baudrate.cend(),
            record.begin() + 32);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 36);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 40);
}

void LinMessage::MakeWakeUp(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 9 : 21;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;

  if (mandatory) {
    return;
  }

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 9);

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 13);
}

void LinMessage::MakeChecksumError(SampleRecord& sample) const {
  MakeDataFrame(sample);
}

void LinMessage::MakeTransmissionError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 10 : 31;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  record[8 + 1] = lin_id_;

  if (mandatory) {
    return;
  }

  record[8 + 2] = spare_;

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 11);

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 15);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 23);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 27);
}

void LinMessage::MakeSyncError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 13 : 29;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 9);

  if (mandatory) {
    return;
  }

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 13);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 21);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 25);
}

void LinMessage::MakeReceiveError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 10 : 43;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  record[8 + 1] = lin_id_;

  if (mandatory) {
    return;
  }

  record[8 + 2] = data_length_;
  record[8 + 3] = crc_;
  record[8 + 4] = spare_;

  for (size_t index = 0; index < 8; ++index) {
    record[13 + index] =
        index < data_bytes_.size() ? data_bytes_[index] : 0xFF;
  }

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(),
            record.begin() + 19);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(),
            record.begin() + 27);

  LittleBuffer response_baudrate(response_baudrate_);
  std::copy(response_baudrate.cbegin(), response_baudrate.cend(),
            record.begin() + 31);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 35);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 39);
}

void LinMessage::MakeSpike(SampleRecord& sample) const {
  MakeWakeUp(sample);
}

void LinMessage::MakeLongDominantSignal(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = writer_.MandatoryMembersOnly();
  const size_t record_size = mandatory ? 10 : 26;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = bus_channel_;
  record[8 + 1] = spare_;

  if (mandatory) {
    return;
  }

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 10);

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 14);

  LittleBuffer length(total_signal_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 22);

}

void LinMessage::ExpectedDataByteCount(uint8_t nof_bytes) {
  spare_ &= ~kExpectedLength;
  spare_ |= nof_bytes & kExpectedLength;
}

uint8_t LinMessage::ExpectedDataByteCount() const {
  return spare_ & kExpectedLength;
}

void LinMessage::TypeOfLongDominantSignal(LinTypeOfLongDominantSignal type) {
  auto temp = static_cast<uint8_t>(type);
  temp <<= 4;
  spare_ &= ~kDominantTypeMask;
  spare_ |= temp;
}

LinTypeOfLongDominantSignal LinMessage::TypeOfLongDominantSignal() const {
  return static_cast<LinTypeOfLongDominantSignal>((spare_ & kDominantTypeMask) >>4);
}

void LinMessage::DataBytes(std::vector<uint8_t>& data_bytes) {
  data_bytes_ = data_bytes;
  DataLength(static_cast<uint8_t>(data_bytes.size()));
}

}  // namespace mdf