/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/ichannelobserver.h"

namespace {
std::string FormatArray(const mdf::IChannelArray& array, uint16_t dimension) {
  const auto dimensions = array.Dimensions();
  std::ostringstream output;
  output << "[";
  output << array.DimensionSize(dimension);
  output << "]";
  ++dimension;
  if (dimension < dimensions) {
    output << FormatArray(array, dimension);
  }
  return output.str();
}

} // empty namespace

namespace mdf {

IChannelObserver::IChannelObserver(const IDataGroup& data_group, const IChannel &channel)
    : ISampleObserver(data_group),
    channel_(channel) {

  // Subscribe on all channel groups
  record_id_list_.clear();
  const auto cg_list = data_group.ChannelGroups();
  record_id_list_.insert(channel_.RecordId());
  if (channel_.VlsdRecordId() > 0 && read_vlsd_data_) {
    record_id_list_.insert(channel_.VlsdRecordId());
  }

}

std::string IChannelObserver::Name() const { return channel_.Name(); }

std::string IChannelObserver::Unit() const {
  return channel_.Unit();
}

const IChannel &IChannelObserver::Channel() const { return channel_; }

bool IChannelObserver::IsMaster() const {
  return channel_.Type() == ChannelType::VirtualMaster ||
         channel_.Type() == ChannelType::Master;
}

bool IChannelObserver::IsArray() const {
  return channel_.ChannelArray() != nullptr;
}

uint64_t IChannelObserver::ArraySize() const {
  const auto* channel_array = channel_.ChannelArray();
  return channel_array != nullptr ? channel_array->NofArrayValues() : 1;
}

std::string IChannelObserver::EngValueToString(uint64_t sample) const {
  bool valid = false;
  std::string value;
  const auto* channel_array = channel_.ChannelArray();
  if (channel_array == nullptr) {
    valid = GetEngValue( sample, value, 0);
    return valid ? value : "*";
  }

  const auto array_size = channel_array->NofArrayValues();
  std::ostringstream output;
  for (auto sample_index = 0; sample_index < array_size; ++sample_index) {
    valid = GetEngValue(sample, value, sample_index);
    if (sample_index > 0) {
      output << ";";
    }
    output << (valid ? value: "*");
  }


  return output.str();
}

void IChannelObserver::ReadVlsdData(bool read_vlsd_data) {
  read_vlsd_data_ = read_vlsd_data;
  // Need to setup the record id subscription list.
  record_id_list_.clear();
  record_id_list_.insert(channel_.RecordId());
  if (channel_.VlsdRecordId() > 0 && read_vlsd_data_) {
    record_id_list_.insert(channel_.VlsdRecordId());
  }
}

std::vector<uint64_t> IChannelObserver::Shape() const {
  const IChannelArray* array = Channel().ChannelArray();
  if (array != nullptr) {
    return array->Shape();
  }
  return {1};
}

template <>
bool IChannelObserver::GetChannelValue(uint64_t sample, std::string& value, uint64_t array_index) const {
  bool valid = false;
  value.clear();
  switch (channel_.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t v = 0;
      valid = GetSampleUnsigned(sample, v, array_index);
      value = std::to_string(v);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t v = 0;
      valid = GetSampleSigned(sample, v, array_index);
      value = std::to_string(v);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double v = 0.0;
      valid = GetSampleFloat(sample, v, array_index);
      value = MdfHelper::FormatDouble(
          v, channel_.IsDecimalUsed() ? channel_.Decimals() : 6);
      break;
    }

    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      valid = GetSampleText(sample, value, array_index);
      break;
    }

    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime: {
      uint64_t ns_since_1970 = 0;
      valid = GetSampleUnsigned(sample, ns_since_1970, array_index);
      value = MdfHelper::NsToLocalIsoTime(ns_since_1970);
      break;
    }
    default:
      break;
  }
  return valid;
}

template <>
bool IChannelObserver::GetChannelValue(uint64_t sample,
                                       std::vector<uint8_t>& value, uint64_t) const {
  bool valid = false;
  value.clear();
  switch (channel_.DataType()) {
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      valid = GetSampleByteArray(sample, value);
      break;
    }

      // Strange to ask for byte array sample if not stored as a byte array.
    default:
      break;
  }
  return valid;
}

template <>
bool IChannelObserver::GetEngValue(uint64_t sample,
                                   std::vector<uint8_t>& value,
                                   uint64_t array_index) const {
  return GetChannelValue(sample, value, array_index );
}
}  // namespace mdf
