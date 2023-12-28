//
// Created by alex on 2023/12/28.
//

#pragma once
#include <algorithm>

#include "mdf/ichannelgroup.h"
#include "mdf/ichannelobserver.h"
#include "mdf/idatagroup.h"
namespace mdf::detail {
template <class T>
class FastChannelObserver : public IChannelObserver {
 private:
  uint64_t record_id_ = 0;
  T value_;
  bool valid_ = false;
  uint64_t current_index = 0;
  const IChannelGroup& group_;

 protected:
  bool GetSampleUnsigned(uint64_t sample, uint64_t& value,
                         uint64_t array_index) const override;

  bool GetSampleSigned(uint64_t sample, int64_t& value,
                       uint64_t array_index) const override;

  bool GetSampleFloat(uint64_t sample, double& value,
                      uint64_t array_index) const override;

  bool GetSampleText(uint64_t sample, std::string& value,
                     uint64_t array_index) const override;

  bool GetSampleByteArray(uint64_t sample,
                          std::vector<uint8_t>& value) const override;

 public:
  FastChannelObserver(const IDataGroup& data_group, const IChannelGroup& group,
                      const IChannel& channel)
      : IChannelObserver(data_group, channel),
        group_(group),
        record_id_(group.RecordId()) {
    const auto* channel_array = channel_.ChannelArray();
    const auto array_size =
        channel_array != nullptr ? channel_array->NofArrayValues() : 1;

    if (channel_.Type() == ChannelType::VariableLength) {
      index_list_.resize(group.NofSamples() * array_size, 0);
    }
    // ChannelObserver::AttachObserver();
  }
  ~FastChannelObserver() override {}
  FastChannelObserver() = delete;
  FastChannelObserver(const FastChannelObserver&) = delete;
  FastChannelObserver(FastChannelObserver&&) = delete;
  FastChannelObserver& operator=(const FastChannelObserver&) = delete;
  FastChannelObserver& operator=(FastChannelObserver&&) = delete;
  [[nodiscard]] uint64_t NofSamples() const override {
    // Note that value_list may be an array.
    return group_.NofSamples();
  }
  void OnSample(uint64_t sample, uint64_t record_id,
                const std::vector<uint8_t>& record) override {
    const auto* channel_array = channel_.ChannelArray();
    auto array_size =
        channel_array != nullptr ? channel_array->NofArrayValues() : 1;

    T value{};
    bool valid;
    current_index =sample;
    switch (channel_.Type()) {
      case ChannelType::VirtualMaster:
      case ChannelType::VirtualData:
        if (record_id_ == record_id) {
          for (uint64_t array_index = 0; array_index < array_size;
               ++array_index) {
            const auto sample_index = (sample * array_size) + array_index;
            valid = channel_.GetVirtualSample(sample, value);
            value_ = value;
            valid_ = valid;
          }
        }
        break;

        // This channel may reference a SD/CG blocks
      case ChannelType::VariableLength:
        if (record_id_ == record_id && channel_.VlsdRecordId() == 0) {
          // If variable length, the value is an index into an SD block.
          for (uint64_t array_index = 0; array_index < array_size;
               ++array_index) {
            uint64_t offset = 0;  // Offset into SD/CG block
            valid = channel_.GetUnsignedValue(record, offset, array_index);

            const auto sample_index = (sample * array_size) + array_index;
            if (sample_index < index_list_.size()) {
              index_list_[sample_index] = offset;
            }
            // Value should be in the channels data list (SD). The channels
            // GetChannelValue handle this situation
            valid = channel_.GetChannelValue(record, value, array_index);
            valid_ = valid;
            value_ = value;
          }
        } else if (channel_.VlsdRecordId() > 0 &&
                   record_id == channel_.VlsdRecordId()) {
          // Add the VLSD offset data to this channel
          for (uint64_t array_index = 0; array_index < array_size;
               ++array_index) {
            const auto sample_index = (sample * array_size) + array_index;
            valid = channel_.GetChannelValue(record, value, array_index);
            valid_ = valid;
            value_ = value;
          }
        }
        break;

      case ChannelType::MaxLength:
      case ChannelType::Sync:
      case ChannelType::Master:
      case ChannelType::FixedLength:
      default:
        if (record_id_ == record_id) {
          for (uint64_t array_index = 0; array_index < array_size;
               ++array_index) {
            const auto sample_index = (sample * array_size) + array_index;
            valid = channel_.GetChannelValue(record, value, array_index);
            valid_ = valid;
            value_ = value;
          }
        }
        break;
    }
  }
};
template <class T>
bool FastChannelObserver<T>::GetSampleUnsigned(uint64_t sample, uint64_t& value,
                                               uint64_t array_index) const {
  if ((current_index) != sample) {
    return false;
  }
  // const auto* channel_array = channel_.ChannelArray();
  // const auto array_size = channel_array != nullptr ?
  // channel_array->NofArrayValues() : 1; const auto sample_index = (sample *
  // array_size) + array_index;
  value = static_cast<uint64_t>(value_);
  return true;
}
// Specialization of the above template function just to keep the compiler
// happy. Fetching an unsigned value as a byte array is not supported.
template <>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleUnsigned(
    uint64_t sample, uint64_t& value, uint64_t array_index) const;
template <>
bool FastChannelObserver<std::string>::GetSampleUnsigned(
    uint64_t sample, uint64_t& value, uint64_t array_index) const;
template <class T>
bool FastChannelObserver<T>::GetSampleSigned(uint64_t sample, int64_t& value,
                                             uint64_t array_index) const {
  if (sample != current_index) {
    return false;
  }
  value = static_cast<int64_t>(value_);
  return valid_;
}
template <>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleSigned(
    uint64_t sample, int64_t& value, uint64_t array_index) const;
template <>
bool FastChannelObserver<std::string>::GetSampleSigned(
    uint64_t sample, int64_t& value, uint64_t array_index) const;
template <class T>
bool FastChannelObserver<T>::GetSampleFloat(uint64_t sample, double& value,
                                            uint64_t array_index) const {
  if (sample != current_index) {
    return false;
  }
  value = static_cast<double>(value_);
  return valid_;
}
template <>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleFloat(
    uint64_t sample, double& value, uint64_t array_index) const;

template <>
bool FastChannelObserver<std::string>::GetSampleFloat(
    uint64_t sample, double& value, uint64_t array_index) const;
template <class T>
bool FastChannelObserver<T>::GetSampleText(uint64_t sample, std::string& value,
                                           uint64_t array_index) const {
  if(sample!=current_index){
    return false;
  }
  try{
    std::ostringstream temp;
    temp<<value_;
    value = temp.str();
    return valid_;
  }
  catch (const std::exception&){}
  return false;
}
template <>
    bool FastChannelObserver<std::vector<uint8_t>>::GetSampleText(
        uint64_t sample, std::string& value, uint64_t  array_index) const;

// Little bit dirty trick but it's the specialized function below that
// normally used.
template <class T>
bool FastChannelObserver<T>::GetSampleByteArray(uint64_t sample,
                                            std::vector<uint8_t>& value) const {
  return false;
}

// This specialized function is actually doing the work with byte arrays
template <>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleByteArray(
    uint64_t sample, std::vector<uint8_t>& value) const;

};  // namespace mdf::detail
