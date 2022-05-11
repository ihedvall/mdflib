/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>
#include <algorithm>
#include "mdf/ichannelobserver.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"

namespace mdf::detail {

template <class T>
class ChannelObserver : public IChannelObserver {
 private:
  uint64_t record_id_ = 0;
  std::vector<T> value_list_;
  std::vector<bool> valid_list_;

  const IDataGroup& data_group_; ///< Reference to the publisher (subject/observer)


  template<typename V>
  bool GetVirtualSample(size_t sample, V& value) const {
    value = static_cast<V>(sample);
    return true;
  }

  template<typename V = std::string>
  bool GetVirtualSample(size_t sample, std::string& value) const {
    value = std::to_string(sample);
    return true;
  }

 protected:
  bool GetSampleUnsigned(size_t sample, uint64_t& value) const override;

  bool GetSampleSigned(uint64_t sample, int64_t& value) const override;

  bool GetSampleFloat(uint64_t sample, double& value) const override;

  bool GetSampleText(uint64_t sample, std::string& value) const override;

  bool GetSampleByteArray(uint64_t sample, std::vector<uint8_t>& value) const override;

 public:
  ChannelObserver(const IDataGroup& data_group, const IChannelGroup& group, const IChannel& channel)
  : IChannelObserver(channel),
    data_group_(data_group),
    record_id_(group.RecordId()),
    value_list_(group.NofSamples(), T {}),
    valid_list_(group.NofSamples(), false) {
    data_group_.AttachSampleObserver(this);
  }
  virtual ~ChannelObserver() {
    data_group_.DetachSampleObserver(this);
  }

  ChannelObserver() = delete;
  ChannelObserver(const ChannelObserver&) = delete;
  ChannelObserver(ChannelObserver&&) = delete;
  ChannelObserver& operator = (const ChannelObserver&) = delete;
  ChannelObserver& operator = (ChannelObserver&&) = delete;

  [[nodiscard]] size_t NofSamples() const override {
    return std::min(valid_list_.size(),value_list_.size());
  }

  void OnSample(size_t sample, uint64_t record_id, const std::vector<uint8_t>& record) override {
    if (record_id_ != record_id ) {
      return;
    }
    switch (channel_.Type()) {
      case ChannelType::VirtualMaster:
      case ChannelType::VirtualData: {
        T value {};
        const bool valid = GetVirtualSample(sample, value);
        if (sample < value_list_.size()) {
          value_list_[sample] = value;
        }
        if (sample < valid_list_.size()) {
          valid_list_[sample] = valid;
        }
        break;
      }

      case ChannelType::MaxLength:
      case ChannelType::Sync:
      case ChannelType::VariableLength:
      case ChannelType::Master:
      case ChannelType::FixedLength:
      default: {
        T value {};
        const bool valid = channel_.GetChannelValue(record, value);
        if (sample < value_list_.size()) {
          value_list_[sample] = value;
        }
        if (sample < valid_list_.size()) {
          valid_list_[sample] = valid;
        }
        break;
      }
    }
  }
};


template<class T>
bool ChannelObserver<T>::GetSampleUnsigned(size_t sample, uint64_t &value) const {
  value = sample < value_list_.size() ? value_list_[sample] : T {};
  return sample < valid_list_.size() && valid_list_[sample];
}


template<>
bool ChannelObserver< std::vector<uint8_t> >::GetSampleUnsigned(size_t sample, uint64_t &value) const {
  value = 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<>
bool ChannelObserver< std::string >::GetSampleUnsigned(size_t sample, uint64_t &value) const {
  value = sample < value_list_.size() ? std::stoull(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}


template<class T>
bool ChannelObserver<T>::GetSampleSigned(uint64_t sample, int64_t &value) const {
  value = sample < value_list_.size() ? value_list_[sample] : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<>
bool ChannelObserver< std::vector<uint8_t> >::GetSampleSigned(size_t sample, int64_t &value) const {
  value = 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<>
bool ChannelObserver< std::string >::GetSampleSigned(size_t sample, int64_t &value) const {
  value = sample < value_list_.size() ? std::stoll(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<class T>
bool ChannelObserver<T>::GetSampleFloat(uint64_t sample, double &value) const {
  value = sample < value_list_.size() ? value_list_[sample] : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<>
bool ChannelObserver< std::vector<uint8_t> >::GetSampleFloat(size_t sample, double &value) const {
  value = 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<>
bool ChannelObserver< std::string >::GetSampleFloat(size_t sample, double &value) const {
  value = sample < value_list_.size() ? std::stod(value_list_[sample]) : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<class T>
bool ChannelObserver<T>::GetSampleText(uint64_t sample, std::string &value) const {
  value = sample < value_list_.size() ? value_list_[sample] : 0;
  return sample < valid_list_.size() && valid_list_[sample];
}

template<>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleText(uint64_t sample, std::string &value) const {
  std::ostringstream s;
  if (sample < value_list_.size()) {
    const auto& list = value_list_[sample];
    for (const auto byte : list) {
      s << std::setfill('0') << std::setw(2) << std::uppercase
        << std::hex << static_cast<uint16_t>(byte);
    }
  }
  value = s.str();
  return sample < valid_list_.size() && valid_list_[sample];
}

template<class T>
bool ChannelObserver<T>::GetSampleByteArray(uint64_t sample, std::vector<uint8_t> &value) const {
  value = {};
  return sample < valid_list_.size() && valid_list_[sample];
}



template<>
bool ChannelObserver< std::vector<uint8_t> >::GetSampleByteArray(uint64_t sample, std::vector<uint8_t> &value) const {
  if (sample < value_list_.size()) {
    value = value_list_[sample];
  }
  return sample < valid_list_.size() && valid_list_[sample];
}


} // namespace mdf::detail