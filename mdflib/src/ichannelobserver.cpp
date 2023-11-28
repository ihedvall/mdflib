/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/ichannelobserver.h"

namespace mdf {

IChannelObserver::IChannelObserver(const IChannel &channel)
    : channel_(channel) {}

std::string IChannelObserver::Name() const { return channel_.Name(); }

std::string IChannelObserver::Unit() const {
  if (channel_.IsUnitValid()) {
    return channel_.Unit();
  }
  const auto *conversion = channel_.ChannelConversion();
  if (conversion != nullptr && conversion->IsUnitValid()) {
    return conversion->Unit();
  }
  return {};
}

const IChannel &IChannelObserver::Channel() const { return channel_; }

bool IChannelObserver::IsMaster() const {
  return channel_.Type() == ChannelType::VirtualMaster ||
         channel_.Type() == ChannelType::Master;
}

template <>
bool IChannelObserver::GetChannelValue(uint64_t sample, std::string& value) const {
  bool valid = false;
  value.clear();
  switch (channel_.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t v = 0;
      valid = GetSampleUnsigned(sample, v);
      value = std::to_string(v);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t v = 0;
      valid = GetSampleSigned(sample, v);
      value = std::to_string(v);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double v = 0.0;
      valid = GetSampleFloat(sample, v);
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
      valid = GetSampleText(sample, value);
      break;
    }

    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime: {
      uint64_t ns_since_1970 = 0;
      valid = GetSampleUnsigned(sample, ns_since_1970);
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
                                       std::vector<uint8_t>& value) const {
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

}  // namespace mdf
