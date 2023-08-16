/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelobserver.h
 * \brief A channel observer is holds a list of channel samples for a
 * channel.
 */
#pragma once
#include <string>
#include <vector>

#include "mdf/ichannel.h"
#include "mdf/isampleobserver.h"
#include "mdf/mdfhelper.h"

namespace mdf {

/** \brief The channel observer object shall hold all samples for a channel.
 *
 * The main purpose for a channel observer is to store all channel samples for
 * a channel. This object is used when reading data from a MDF file.
 */
class IChannelObserver : public ISampleObserver {
 protected:
  const IChannel& channel_; ///< Reference to the channel (CN) block.
  std::vector<uint64_t> index_list_; ///< Only used for VLSD channels.
  virtual bool GetSampleUnsigned(uint64_t sample, uint64_t& value)
      const = 0; ///< Returns a unsigned  sample value.
  virtual bool GetSampleSigned(uint64_t sample, int64_t& value)
      const = 0; ///< Returns a signed sample value.
  virtual bool GetSampleFloat(uint64_t sample, double& value)
      const = 0; ///< Returns a float sample value.
  virtual bool GetSampleText(uint64_t sample, std::string& value)
      const = 0; ///< Returns a string sample value.
  virtual bool GetSampleByteArray(uint64_t sample, std::vector<uint8_t>& value)
      const = 0; ///< Returns a byte array sample value.

 public:
  explicit IChannelObserver(const IChannel& channel); ///< Constructor.

  ~IChannelObserver() override = default; ///< Default destructor.

  IChannelObserver() = delete;
  IChannelObserver(const IChannelObserver&) = delete;
  IChannelObserver(IChannelObserver&&) = delete;
  IChannelObserver& operator=(const IChannelObserver&) = delete;
  IChannelObserver& operator=(IChannelObserver&&) = delete;

  [[nodiscard]] virtual uint64_t NofSamples()
      const = 0; ///< Returns number of samples.

  [[nodiscard]] std::string Name() const; ///< Channel name

  [[nodiscard]] std::string Unit() const; ///< Channel unit.

  [[nodiscard]] const IChannel& Channel() const; ///< Channel.
  [[nodiscard]] bool IsMaster() const; ///< True if this is the master channel.

  /** \brief Returns the channel value for a sample.
   *
   * Returns the (unscaled) channel value for a specific sample.
   * @tparam V Type of value
   * @param sample Sample number (0..).
   * @param value The channel value.
   * @return True if value is valid.
   */
  template <typename V>
  bool GetChannelValue(uint64_t sample, V& value) const;

  /** \brief Returns the engineering value for a specific value.
   *
   * Returns the engineering (scaled) value for a specific value.
   * @tparam V Type of return value
   * @param sample Sample number (0..).
   * @param value The return value.
   * @return True if the value is valid.
   */
  template <typename V>
  bool GetEngValue(uint64_t sample, V& value) const;
};

template <typename V>
bool IChannelObserver::GetChannelValue(uint64_t sample, V& value) const {
  bool valid = false;
  value = {};
  switch (channel_.DataType()) {
    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime: {
      uint64_t v = 0;  // ns since 1970
      valid = GetSampleUnsigned(sample, v);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t v = 0;
      valid = GetSampleUnsigned(sample, v);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t v = 0;
      valid = GetSampleSigned(sample, v);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double v = 0.0;
      valid = GetSampleFloat(sample, v);
      value = static_cast<V>(v);
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
      value = static_cast<V>(v.empty() ? V{} : v[0]);
      break;
    }

    default:
      break;
  }
  return valid;
}

/** \brief Returns the sample as a string. */
template <>
bool IChannelObserver::GetChannelValue(uint64_t sample,
                                       std::string& value) const;

/** \brief Returns the sample as a byte. */
template <>
bool IChannelObserver::GetChannelValue(uint64_t sample,
                                       std::vector<uint8_t>& value) const;


template <typename V>
bool IChannelObserver::GetEngValue(uint64_t sample, V& value) const {
  const auto* conversion = channel_.ChannelConversion();
  if (conversion == nullptr) {
    return GetChannelValue(sample, value);
  }

  bool valid = false;
  value = {};
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
      valid = GetChannelValue(sample, value);  // No conversion is allowed;
      break;

    default:
      break;
  }
  return valid;
}

}  // namespace mdf
