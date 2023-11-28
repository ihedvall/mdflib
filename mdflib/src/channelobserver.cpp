/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "channelobserver.h"

namespace mdf::detail {

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleUnsigned(
    uint64_t sample, uint64_t &value) const {
  value = 0; // value_list is an byte array
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::string>::GetSampleUnsigned(uint64_t sample,
                                                     uint64_t &value) const {
  // Convert the string to an unsigned value
  value = sample < value_list_.size() ? std::stoull(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleSigned(
    uint64_t sample, int64_t &value) const {
  value = 0; // value_list is an byte array
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::string>::GetSampleSigned(uint64_t sample,
                                                   int64_t &value) const {
  value = sample < value_list_.size() ? std::stoll(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleFloat(
    uint64_t sample, double &value) const {
  value = 0.0; // value_list is an byte array
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::string>::GetSampleFloat(uint64_t sample,
                                                  double &value) const {
  value = sample < value_list_.size() ? std::stod(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleText(
    uint64_t sample, std::string &value) const {
  // The value_list consist of byte arrays. Convert o
  std::ostringstream s;
  if (sample < value_list_.size()) {
    const auto &list = value_list_[sample];
    for (const auto byte : list) {
      s << std::setfill('0') << std::setw(2) << std::uppercase << std::hex
        << static_cast<uint16_t>(byte);
    }
  }
  value = s.str();
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleByteArray(
    uint64_t sample, std::vector<uint8_t> &value) const {
  if (sample < value_list_.size()) {
    value = value_list_[sample];
  }
  return sample < valid_list_.size() && valid_list_[sample];
}

}  // namespace mdf::detail
