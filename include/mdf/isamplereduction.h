/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iblock.h"
#include <string>
#include "mdf/ichannel.h"
namespace mdf {
class IChannelGroup;


enum class SrSyncType : uint8_t {
  Undefined = 0,
  Time = 1,
  Angle = 2,
  Distance = 3,
  Index = 4,
};

namespace SrFlag {

constexpr uint8_t InvalidationByte = 0x01;
constexpr uint8_t DominantBit = 0x02;

} // End namespace SrFlag

template <typename T>
struct SrValue {
  T MeanValue = {};
  T MinValue = {};
  T MaxValue = {};
  bool MeanValid = false;
  bool MinValid = false;
  bool MaxValid = false;
};

class ISampleReduction : public IBlock {
public:

  virtual void NofSamples(uint64_t nof_samples) = 0;
  [[nodiscard]] virtual uint64_t NofSamples() const = 0;

  virtual void Interval(double interval) = 0;
  [[nodiscard]] virtual double Interval() const = 0;

  virtual void SyncType(SrSyncType type);
  [[nodiscard]] virtual SrSyncType SyncType() const;

  virtual void Flags(uint8_t flags);
  [[nodiscard]] virtual uint8_t Flags() const;

  [[nodiscard]] virtual const IChannelGroup* ChannelGroup() const = 0;

  template <typename T>
  void GetChannelValue( const IChannel& channel, uint64_t sample,
  uint64_t array_index, SrValue<T>& value ) const;

  template <typename T>
  void GetEngValue( const IChannel& channel, uint64_t sample,
                    uint64_t array_index, SrValue<T>& value ) const;

  virtual void ClearData() = 0;

 protected:
  virtual void GetChannelValueUint( const IChannel& channel, uint64_t sample,
                                uint64_t array_index, SrValue<uint64_t>& value ) const = 0;
  virtual void GetChannelValueInt( const IChannel& channel, uint64_t sample,
                                uint64_t array_index, SrValue<int64_t>& value ) const = 0;
  virtual void GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                                uint64_t array_index, SrValue<double>& value ) const = 0;
};

template<typename T>
void ISampleReduction::GetChannelValue( const IChannel& channel, uint64_t sample,
                                        uint64_t array_index, SrValue<T>& value ) const {
  value = {};
  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      SrValue<uint64_t> temp;
      GetChannelValueUint(channel, sample, array_index, temp);
      value.MeanValue = static_cast<T>(temp.MeanValue);
      value.MinValue = static_cast<T>(temp.MinValue);
      value.MaxValue = static_cast<T>(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      SrValue<int64_t> temp;
      GetChannelValueInt(channel, sample, array_index, temp);
      value.MeanValue = static_cast<T>(temp.MeanValue);
      value.MinValue = static_cast<T>(temp.MinValue);
      value.MaxValue = static_cast<T>(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      SrValue<double> temp;
      GetChannelValueDouble(channel, sample, array_index, temp);
      value.MeanValue = static_cast<T>(temp.MeanValue);
      value.MinValue = static_cast<T>(temp.MinValue);
      value.MaxValue = static_cast<T>(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    default:
      break;
  }
}

template<>
void ISampleReduction::GetChannelValue( const IChannel& channel, uint64_t sample,
                 uint64_t array_index, SrValue<std::string>& value) const;


template<typename T>
void ISampleReduction::GetEngValue( const IChannel& channel, uint64_t sample,
                                        uint64_t array_index, SrValue<T>& value ) const {
  value = {};

  const auto* channel_conversion = channel.ChannelConversion();

  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      SrValue<uint64_t> temp;
      GetChannelValueUint(channel, sample, array_index, temp);
      if (channel_conversion != 0) {
        const bool mean_valid = channel_conversion->Convert(temp.MeanValue, value.MeanValue);
        const bool min_valid = channel_conversion->Convert(temp.MinValue, value.MinValue);
        const bool max_valid = channel_conversion->Convert(temp.MaxValue, value.MaxValue);
        value.MeanValid = temp.MeanValid && mean_valid;
        value.MinValid = temp.MinValid && min_valid;
        value.MaxValid = temp.MaxValid && max_valid;
      } else {
        value.MeanValue = static_cast<T>(temp.MeanValue);
        value.MinValue = static_cast<T>(temp.MinValue);
        value.MaxValue = static_cast<T>(temp.MaxValue);
        value.MeanValid = temp.MeanValid;
        value.MinValid = temp.MinValid;
        value.MaxValid = temp.MaxValid;
      }
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      SrValue<int64_t> temp;
      GetChannelValueInt(channel, sample, array_index, temp);
      if (channel_conversion != 0) {
        const bool mean_valid = channel_conversion->Convert(temp.MeanValue, value.MeanValue);
        const bool min_valid = channel_conversion->Convert(temp.MinValue, value.MinValue);
        const bool max_valid = channel_conversion->Convert(temp.MaxValue, value.MaxValue);
        value.MeanValid = temp.MeanValid && mean_valid;
        value.MinValid = temp.MinValid && min_valid;
        value.MaxValid = temp.MaxValid && max_valid;
      } else {
        value.MeanValue = static_cast<T>(temp.MeanValue);
        value.MinValue = static_cast<T>(temp.MinValue);
        value.MaxValue = static_cast<T>(temp.MaxValue);
        value.MeanValid = temp.MeanValid;
        value.MinValid = temp.MinValid;
        value.MaxValid = temp.MaxValid;
      }
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      SrValue<double> temp;
      GetChannelValueDouble(channel, sample, array_index, temp);
      if (channel_conversion != 0) {
        const bool mean_valid = channel_conversion->Convert(temp.MeanValue, value.MeanValue);
        const bool min_valid = channel_conversion->Convert(temp.MinValue, value.MinValue);
        const bool max_valid = channel_conversion->Convert(temp.MaxValue, value.MaxValue);
        value.MeanValid = temp.MeanValid && mean_valid;
        value.MinValid = temp.MinValid && min_valid;
        value.MaxValid = temp.MaxValid && max_valid;
      } else {
        value.MeanValue = static_cast<T>(temp.MeanValue);
        value.MinValue = static_cast<T>(temp.MinValue);
        value.MaxValue = static_cast<T>(temp.MaxValue);
        value.MeanValid = temp.MeanValid;
        value.MinValid = temp.MinValid;
        value.MaxValid = temp.MaxValid;
      }
      break;
    }

    default:
      break;
  }
}

template<>
void ISampleReduction::GetEngValue( const IChannel& channel, uint64_t sample,
                                    uint64_t array_index, SrValue<std::string>& value ) const;
} // mdf
