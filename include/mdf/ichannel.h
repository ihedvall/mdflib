/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <iomanip>
#include "mdf/ichannelconversion.h"
#include "mdf/mdfhelper.h"

namespace mdf {

enum class ChannelType : uint8_t {
  FixedLength = 0,
  VariableLength = 1,
  Master = 2,
  VirtualMaster = 3,
  Sync = 4,
  MaxLength = 5,
  VirtualData = 6
};

enum class ChannelSyncType : uint8_t {
  None = 0,
  Time = 1,
  Angle = 2,
  Distance = 3,
  Index = 4
};

enum class ChannelDataType : uint8_t {
  UnsignedIntegerLe = 0,
  UnsignedIntegerBe = 1,
  SignedIntegerLe = 2,
  SignedIntegerBe = 3,
  FloatLe = 4,
  FloatBe = 5,
  StringAscii = 6,
  StringUTF8 = 7,
  StringUTF16Le = 8,
  StringUTF16Be = 9,
  ByteArray = 10,
  MimeSample = 11,
  MimeStream = 12,
  CanOpenDate = 13,
  CanOpenTime = 14,
  ComplexLe = 15,
  ComplexBE = 16
};

namespace CnFlag {
  constexpr uint32_t AllValuesInvalid = 0x0001;
  constexpr uint32_t InvalidValid = 0x0002;
  constexpr uint32_t PrecisionValid = 0x0004;
  constexpr uint32_t RangeValid = 0x0008;
  constexpr uint32_t LimitValid = 0x0010;
  constexpr uint32_t ExtendedLimitValid = 0x0020;
  constexpr uint32_t Discrete = 0x0040;
  constexpr uint32_t Calibration = 0x0080;
  constexpr uint32_t Calculated = 0x0100;
  constexpr uint32_t Virtual = 0x0200;
  constexpr uint32_t BusEvent = 0x0400;
  constexpr uint32_t StrictlyMonotonous = 0x0800;
  constexpr uint32_t DefaultX = 0x1000;
  constexpr uint32_t EventSignal = 0x2000;
  constexpr uint32_t VlsdDataStream = 0x4000;
}

class IChannel {

 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void Name(const std::string& name) = 0;
  [[nodiscard]] virtual std::string Name() const = 0;

  virtual void DisplayName(const std::string& name) = 0;
  [[nodiscard]] virtual std::string DisplayName() const = 0;

  virtual void Description(const std::string& description) = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  virtual void Unit(const std::string& unit) = 0;
  [[nodiscard]] virtual std::string Unit() const = 0;

  [[nodiscard]] virtual bool IsUnitValid() const = 0;

  virtual void Type(ChannelType type) = 0;
  [[nodiscard]] virtual ChannelType Type() const = 0;

  virtual void Sync(ChannelSyncType type);
  [[nodiscard]] virtual ChannelSyncType Sync() const;

  virtual void DataType(ChannelDataType type) = 0;
  [[nodiscard]] virtual ChannelDataType DataType() const = 0;

  virtual void DataBytes(size_t nof_bytes) = 0;
  [[nodiscard]] virtual size_t DataBytes() const = 0;

  [[nodiscard]] virtual uint8_t Decimals() const = 0;
  [[nodiscard]] virtual bool IsDecimalUsed() const = 0;

  virtual void Range(double min, double max);
  [[nodiscard]] virtual std::optional<std::pair<double, double>> Range() const;

  virtual void Limit(double min, double max);
  [[nodiscard]] virtual std::optional<std::pair<double, double>> Limit() const;

  virtual void ExtLimit(double min, double max);
  [[nodiscard]] virtual std::optional<std::pair<double, double>> ExtLimit() const;

  virtual void SamplingRate(double sampling_rate) = 0;
  [[nodiscard]] virtual double SamplingRate() const = 0;

  [[nodiscard]] virtual const IChannelConversion *ChannelConversion() const = 0;

  [[nodiscard]] bool IsNumber() const {
    // Need to check the cc at well if it is a value to text conversion
    const auto* cc = ChannelConversion();
    if (cc != nullptr && cc->Type() <= ConversionType::ValueRangeToValue) {
      return true;
    }
    return DataType() <= ChannelDataType::FloatBe;
  }

  template<typename T>
  bool GetChannelValue(const std::vector<uint8_t> &record_buffer, T &dest) const;

  template<typename T>
  void SetChannelValue(const T& value, bool valid = true) {
    switch (DataType()) {
      case ChannelDataType::UnsignedIntegerLe:
        SetUnsignedValueLe(static_cast<uint64_t>(value), valid);
        break;

     case ChannelDataType::UnsignedIntegerBe:
        SetUnsignedValueBe(static_cast<uint64_t>(value), valid);
        break;


      case ChannelDataType::SignedIntegerLe:
        SetSignedValueLe(static_cast<int64_t>(value), valid);
        break;

      case ChannelDataType::SignedIntegerBe:
        SetSignedValueBe(static_cast<int64_t>(value), valid);
        break;

      case ChannelDataType::FloatLe:
        SetFloatValueLe(static_cast<double>(value), valid);
        break;

      case ChannelDataType::FloatBe:
        SetFloatValueBe(static_cast<double>(value), valid);
        break;

      case ChannelDataType::StringUTF8:
      case ChannelDataType::StringAscii:
        SetTextValue(std::to_string(value), valid);
        break;

      case ChannelDataType::ByteArray:
        if (typeid(T) == typeid(uint64_t)) {
          if (DataBytes() == 7) {
            auto date_array = MdfHelper::NsToCanOpenDateArray(static_cast<uint64_t>(value));
            SetByteArray(date_array,valid);
          } else if (DataBytes() == 6) {
            auto time_array = MdfHelper::NsToCanOpenTimeArray(static_cast<uint64_t>(value));
            SetByteArray(time_array,valid);
          }
        } else {
          SetValid(false);
        }
        break;

      case ChannelDataType::CanOpenDate:
        if (typeid(T) == typeid(uint64_t) && DataBytes() == 7) {
          const auto date_array = MdfHelper::NsToCanOpenDateArray(static_cast<uint64_t>(value));
          SetByteArray(date_array,valid);
        } else {
          SetValid(false);
        }
        break;

      case ChannelDataType::CanOpenTime:
        if (typeid(T) == typeid(uint64_t) && DataBytes() == 6) {
          const auto time_array = MdfHelper::NsToCanOpenTimeArray(static_cast<uint64_t>(value));
          SetByteArray(time_array,valid);
        } else {
          SetValid(false);
        }
        break;

      case ChannelDataType::StringUTF16Le:
      case ChannelDataType::StringUTF16Be:
      case ChannelDataType::MimeStream:
      case ChannelDataType::MimeSample:
      default:
        SetValid(false);
        break;
    }
  };

  template<typename T = std::string>
  void SetChannelValue(const std::string& value, bool valid = true);

  template<typename T = std::vector<uint8_t>>
  void SetChannelValue(const std::vector<uint8_t>& value, bool valid = true);
 protected:
  [[nodiscard]] virtual size_t BitCount() const = 0;   ///< Returns number of bits in value.
  [[nodiscard]] virtual size_t BitOffset() const = 0;  ///< Returns bit offset (0..7).
  [[nodiscard]] virtual size_t ByteOffset() const = 0; ///< Returns byte offset in record.


  virtual void CopyToDataBuffer(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t>& data_buffer) const;
  virtual bool GetUnsignedValue(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const;
  virtual bool GetSignedValue(const std::vector<uint8_t> &record_buffer, int64_t &dest) const;
  virtual bool GetFloatValue(const std::vector<uint8_t> &record_buffer, double &dest) const;
  virtual bool GetTextValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const;
  virtual bool GetByteArrayValue(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t> &dest) const;
  virtual bool GetCanOpenDate(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const;
  virtual bool GetCanOpenTime(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const;

  [[nodiscard]] virtual std::vector<uint8_t>& SampleBuffer() const = 0;

  virtual void SetValid(bool valid);

  void SetUnsignedValueLe(uint64_t value, bool valid);
  void SetUnsignedValueBe(uint64_t value, bool valid);
  void SetSignedValueLe(int64_t value, bool valid);
  void SetSignedValueBe(int64_t value, bool valid);
  void SetFloatValueLe(double value, bool valid);
  void SetFloatValueBe(double value, bool valid);
  void SetTextValue(const std::string& value, bool valid);
  void SetByteArray(const std::vector<uint8_t>& value, bool valid);
  std::vector<uint8_t> NsToDateArray(uint64_t ns_since_1970) const;
};

template<typename T>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer, T &dest) const {
  bool valid = false;
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t value = 0;
      valid = GetUnsignedValue(record_buffer, value);
      dest = static_cast<T>(value);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t value = 0;
      valid = GetSignedValue(record_buffer, value);
      dest = static_cast<T>(value);
      break;
    }
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double value = 0;
      valid = GetFloatValue(record_buffer, value);
      dest = static_cast<T>(value);
      break;
    }

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii: {
      std::string text;
      valid = GetTextValue(record_buffer, text);
      std::istringstream data(text);
      data >> dest;
      break;
    }

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      std::vector<uint8_t> list;
      valid = GetByteArrayValue(record_buffer, list);
      dest = list.empty() ? T{} : list[0];
      break;
    }

    case ChannelDataType::CanOpenDate: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenDate(record_buffer, ms_since_1970);
      dest = static_cast<T>(ms_since_1970);
      break;
    }

    case ChannelDataType::CanOpenTime: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenTime(record_buffer, ms_since_1970);
      dest = static_cast<T>(ms_since_1970);
      break;
    }
    default: break;
  }
  return valid;
}

template<>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t> &dest) const;

template<>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const;


}
