/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include "mdf/isampleobserver.h"
#include "mdf/ichannel.h"
#include "util/logstream.h"

namespace mdf {

class IChannelObserver : public ISampleObserver
{
 protected:
  const IChannel& channel_;
  virtual bool GetSampleUnsigned(uint64_t sample, uint64_t& value) const = 0;
  virtual bool GetSampleSigned(uint64_t sample, int64_t& value) const = 0;
  virtual bool GetSampleFloat(uint64_t sample, double& value) const = 0;
  virtual bool GetSampleText(uint64_t sample, std::string& value) const = 0;
  virtual bool GetSampleByteArray(uint64_t sample, std::vector<uint8_t>& value) const = 0;
 public:
  explicit IChannelObserver(const IChannel& channel);

  ~IChannelObserver() override = default;

  IChannelObserver() = delete;
  IChannelObserver(const IChannelObserver&) = delete;
  IChannelObserver(IChannelObserver&&) = delete;
  IChannelObserver& operator = (const IChannelObserver&) = delete;
  IChannelObserver& operator = (IChannelObserver&&) = delete;

  [[nodiscard]] virtual size_t NofSamples() const = 0;

  [[nodiscard]] std::string Name() const;

  [[nodiscard]] std::string Unit() const;

  [[nodiscard]] const IChannel& Channel() const;
  [[nodiscard]] bool IsMaster() const;

  template<typename V>
  bool GetChannelValue(size_t sample, V &value) const {
    bool valid = false;
    value  = {};
    switch (channel_.DataType()) {
      case ChannelDataType::CanOpenDate:
      case ChannelDataType::CanOpenTime: {
        uint64_t v = 0; // ns since 1970
        valid = GetSampleUnsigned(sample, v);
        value = static_cast<V> (v);
        break;
      }

      case ChannelDataType::UnsignedIntegerLe:
      case ChannelDataType::UnsignedIntegerBe: {
        uint64_t v = 0;
        valid = GetSampleUnsigned(sample, v);
        value = static_cast<V> (v);
        break;
      }

      case ChannelDataType::SignedIntegerLe:
      case ChannelDataType::SignedIntegerBe: {
        int64_t v = 0;
        valid = GetSampleSigned(sample, v);
        value = static_cast<V> (v);
        break;
      }

      case ChannelDataType::FloatLe:
      case ChannelDataType::FloatBe: {
        double v = 0.0;
        valid = GetSampleFloat(sample, v);
        value = static_cast<V> (v);
        break;
      }

      case ChannelDataType::StringAscii:
      case ChannelDataType::StringUTF8:
      case ChannelDataType::StringUTF16Le:
      case ChannelDataType::StringUTF16Be: {
        std::string v;
        valid = GetSampleText(sample, v);
        std::istringstream s(v);
        s >> value;
        break;
      }

      case ChannelDataType::MimeStream:
      case ChannelDataType::MimeSample:
      case ChannelDataType::ByteArray: {
        std::vector<uint8_t> v;
        valid = GetSampleByteArray(sample, v);
        value = static_cast<V> (v.empty() ? V {} : v[0]);
        break;
      }

      default: break;
    }
    return valid;
  }

  template<typename V = std::string>
  bool GetChannelValue(uint64_t sample, std::string &value) const {
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
        value = util::string::FormatDouble(v,channel_.IsDecimalUsed() ? channel_.Decimals() : 6);
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
        value = util::time::NsToLocalIsoTime(ns_since_1970);
        break;
      }
      default: break;
    }
    return valid;
  }

  template<typename V = std::vector<uint8_t>>
  bool GetChannelValue(uint64_t sample, std::vector<uint8_t> &value) const {
    bool valid = false;
    value.clear();
    switch (channel_.DataType()) {
      case ChannelDataType::ByteArray: {
        valid = GetSampleByteArray(sample, value);
        break;
      }

      default: break;
    }
    return valid;
  }

  template <typename V>
  bool GetEngValue(size_t sample, V& value) const {
    const auto* conversion = channel_.ChannelConversion();
    if (conversion == nullptr) {
      return GetChannelValue(sample, value);
    }

    bool valid = false;
    value  = {};
    switch (channel_.DataType()) {
      case ChannelDataType::UnsignedIntegerLe:
      case ChannelDataType::UnsignedIntegerBe: {
        uint64_t v = 0;
        valid = GetSampleUnsigned(sample, v) && conversion->Convert(v, value);
        break;
      }

      case ChannelDataType::SignedIntegerLe:
      case ChannelDataType::SignedIntegerBe: {
        int64_t v = 0;
        valid = GetSampleSigned(sample, v) && conversion->Convert(v, value);
        break;
      }

      case ChannelDataType::FloatLe:
      case ChannelDataType::FloatBe: {
        double v = 0.0;
        valid = GetSampleFloat(sample, v) && conversion->Convert(v, value);
        break;
      }

      case ChannelDataType::CanOpenDate:
      case ChannelDataType::CanOpenTime:
        valid = GetChannelValue(sample, value); // No conversion is allowed;
        break;

      default: break;
    }
    return valid;
  }

};


} // namespace mdf
