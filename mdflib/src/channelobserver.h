/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <algorithm>
#include <vector>

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/ichannelobserver.h"
#include "mdf/idatagroup.h"

namespace mdf::detail {

template <class T>
class ChannelObserver : public IChannelObserver {
 private:
  uint64_t record_id_ = 0;
  std::vector<T> value_list_;
  std::vector<bool> valid_list_;

  const IDataGroup&
      data_group_;  ///< Reference to the publisher (subject/observer)

  template <typename V>
  bool GetVirtualSample(uint64_t sample, V& value) const {
    value = static_cast<V>(sample);
    return true;
  }

  template <typename V = std::string>
  bool GetVirtualSample(uint64_t sample, std::string& value) const {
    value = std::to_string(sample);
    return true;
  }

 protected:
  bool GetSampleUnsigned(uint64_t sample, uint64_t& value) const override;

  bool GetSampleSigned(uint64_t sample, int64_t& value) const override;

  bool GetSampleFloat(uint64_t sample, double& value) const override;

  bool GetSampleText(uint64_t sample, std::string& value) const override;

  bool GetSampleByteArray(uint64_t sample,
                          std::vector<uint8_t>& value) const override;

 public:
  ChannelObserver(const IDataGroup& data_group, const IChannelGroup& group,
                  const IChannel& channel)
      : IChannelObserver(channel),
        data_group_(data_group),
        record_id_(group.RecordId()),
        value_list_(group.NofSamples(), T{}),
        valid_list_(group.NofSamples(), false) {
    if (channel_.Type() == ChannelType::VariableLength) {
      index_list_.resize(group.NofSamples());
    }
    data_group_.AttachSampleObserver(this);
  }
  ~ChannelObserver() override { data_group_.DetachSampleObserver(this); }

  ChannelObserver() = delete;
  ChannelObserver(const ChannelObserver&) = delete;
  ChannelObserver(ChannelObserver&&) = delete;
  ChannelObserver& operator=(const ChannelObserver&) = delete;
  ChannelObserver& operator=(ChannelObserver&&) = delete;

  [[nodiscard]] uint64_t NofSamples() const override {
    return std::min(valid_list_.size(), value_list_.size());
  }

  void OnSample(uint64_t sample, uint64_t record_id,
                const std::vector<uint8_t>& record) override {

    switch (channel_.Type()) {

      case ChannelType::VirtualMaster:
      case ChannelType::VirtualData:
        if (record_id_ == record_id) {
          T value{};
          const bool valid = GetVirtualSample(sample, value);
          if (sample < value_list_.size()) {
            value_list_[sample] = value;
          }
          if (sample < valid_list_.size()) {
            valid_list_[sample] = valid;
          }
        }
        break;

        // This channel may reference a CG blocks another record id
      case ChannelType::VariableLength: {
        if (record_id_ == record_id && channel_.VlsdRecordId() == 0) {
          uint64_t index = 0;
          bool valid = channel_.GetUnsignedValue(record, index);
          if (sample < index_list_.size()) {
            index_list_[sample] = index;
          }
          T value;
          valid = channel_.GetChannelValue(record, value);
          if (sample < valid_list_.size()) {
            valid_list_[sample] = valid;
          }
          if (sample < value_list_.size()) {
            value_list_[sample] = value;
          }
        } else if (channel_.VlsdRecordId() > 0 &&
                   record_id == channel_.VlsdRecordId()) {
          // Add the VLSD sample data to this channel
          T value{};
          const bool valid = channel_.GetChannelValue(record, value);
          if (sample < value_list_.size()) {
            value_list_[sample] = value;
          }
          if (sample < valid_list_.size()) {
            valid_list_[sample] = valid;
          }
        }
      }
        break;


      case ChannelType::MaxLength:
      case ChannelType::Sync:
      case ChannelType::Master:
      case ChannelType::FixedLength:
      default:
        if (record_id_ == record_id) {
          T value{};
          const bool valid = channel_.GetChannelValue(record, value);
          if (sample < value_list_.size()) {
            value_list_[sample] = value;
          }
          if (sample < valid_list_.size()) {
            valid_list_[sample] = valid;
          }
        }
        break;
    }
  }
};

template <class T>
bool ChannelObserver<T>::GetSampleUnsigned(uint64_t sample,
                                           uint64_t& value) const {
  value = sample < value_list_.size() ? 
      static_cast<uint64_t>(value_list_[sample]) : static_cast<uint64_t>(T{});
  return sample < valid_list_.size() && valid_list_[sample];
}

// specialization of the above template function just to keep the compiler
// happy. Fetching an unsigned value as a byte array is not supported.
template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleUnsigned(
    uint64_t sample, uint64_t& value) const;

template <>
bool ChannelObserver<std::string>::GetSampleUnsigned(uint64_t sample,
                                                     uint64_t& value) const;

template <class T>
bool ChannelObserver<T>::GetSampleSigned(uint64_t sample,
                                         int64_t& value) const {
  value = sample < value_list_.size() ? static_cast<int64_t>(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleSigned(
    uint64_t sample, int64_t& value) const;

template <>
bool ChannelObserver<std::string>::GetSampleSigned(uint64_t sample,
                                                   int64_t& value) const;

template <class T>
bool ChannelObserver<T>::GetSampleFloat(uint64_t sample, double& value) const {
  value = sample < value_list_.size() ? static_cast<double>(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleFloat(uint64_t sample,
                                                           double& value) const;

template <>
bool ChannelObserver<std::string>::GetSampleFloat(uint64_t sample,
                                                  double& value) const;

template <class T>
bool ChannelObserver<T>::GetSampleText(uint64_t sample,
                                       std::string& value) const {
  std::ostringstream temp;
  if (sample < value_list_.size()) {
    temp << value_list_[sample];
  }

  value =  temp.str();
  return sample < valid_list_.size() && valid_list_[sample];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleText(
    uint64_t sample, std::string& value) const;

// Little bit dirty trick but it's the specialized function below that
// normally used.
template <class T>
bool ChannelObserver<T>::GetSampleByteArray(uint64_t sample,
                                            std::vector<uint8_t>& value) const {
  value = {};
  return sample < valid_list_.size() && valid_list_[sample];
}

// This specialized function is actually doing the work with byte arrays
template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleByteArray(
    uint64_t sample, std::vector<uint8_t>& value) const;

}  // namespace mdf::detail