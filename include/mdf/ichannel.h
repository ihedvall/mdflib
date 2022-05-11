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
#include "util/stringutil.h"
#include "util/timestamp.h"
#include "mdf/ichannelconversion.h"

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
  CanOpenTime = 14
};

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

  virtual void DataType(ChannelDataType type) = 0;
  [[nodiscard]] virtual ChannelDataType DataType() const = 0;

  virtual void DataBytes(size_t nof_bytes) = 0;
  [[nodiscard]] virtual size_t DataBytes() const = 0;

  [[nodiscard]] virtual uint8_t Decimals() const = 0;
  [[nodiscard]] virtual bool IsDecimalUsed() const = 0;

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
  bool GetChannelValue(const std::vector<uint8_t> &record_buffer, T &dest) const {
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
  };

  template<typename T = std::string>
  bool GetChannelValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const {
    bool valid = false;
    switch (DataType()) {
      case ChannelDataType::UnsignedIntegerLe:
      case ChannelDataType::UnsignedIntegerBe: {
        uint64_t value = 0;
        valid = GetUnsignedValue(record_buffer, value);
        std::ostringstream s;
        s << value;
        dest = s.str();
        break;
      }

      case ChannelDataType::SignedIntegerLe:
      case ChannelDataType::SignedIntegerBe: {
        int64_t value = 0;
        valid = GetSignedValue(record_buffer, value);
        dest = std::to_string(value);
        break;
      }
      case ChannelDataType::FloatLe:
      case ChannelDataType::FloatBe: {
        double value = 0;
        valid = GetFloatValue(record_buffer, value);
        dest = IsDecimalUsed() ? util::string::FormatDouble(value, Decimals()) : std::to_string(value);
        break;
      }

      case ChannelDataType::StringUTF16Le:
      case ChannelDataType::StringUTF16Be:
      case ChannelDataType::StringUTF8:
      case ChannelDataType::StringAscii: {
        valid = GetTextValue(record_buffer, dest);
        break;
      }

      case ChannelDataType::MimeStream:
      case ChannelDataType::MimeSample:
      case ChannelDataType::ByteArray: {
        std::vector<uint8_t> list;
        valid = GetByteArrayValue(record_buffer, list);
        std::ostringstream s;
        for (const auto byte: list) {
          s << std::setfill('0') << std::setw(2)
            << std::hex << std::uppercase
            << static_cast<uint16_t>(byte);
        }
        dest = s.str();
        break;
      }

      case ChannelDataType::CanOpenDate: {
        uint64_t ms_since_1970 = 0;
        valid = GetCanOpenDate(record_buffer, ms_since_1970);

        const auto ms = ms_since_1970 % 1000;
        const auto time = static_cast<time_t>(ms_since_1970 / 1000);
        struct tm bt{};
        localtime_s(&bt, &time);

        std::ostringstream text;
        text << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
             << '.' << std::setfill('0') << std::setw(3) << ms;
        dest = text.str();
        break;
      }

      case ChannelDataType::CanOpenTime: {
        uint64_t ms_since_1970 = 0;
        valid = GetCanOpenTime(record_buffer, ms_since_1970);

        const auto ms = ms_since_1970 % 1000;
        const auto time = static_cast<time_t>(ms_since_1970 / 1000);
        struct tm bt{};
        localtime_s(&bt, &time);

        std::ostringstream text;
        text << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
             << '.' << std::setfill('0') << std::setw(3) << ms;
        dest = text.str();
        break;
      }

      default: break;
    }
    return valid;
  };

  template<typename T = std::vector<uint8_t>>
  bool GetChannelValue(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t> &dest) const {
    bool valid = false;
    switch (DataType()) {
      case ChannelDataType::UnsignedIntegerLe:
      case ChannelDataType::UnsignedIntegerBe: {
        uint64_t value = 0;
        valid = GetUnsignedValue(record_buffer, value);
        dest.resize(1);
        dest[0] = static_cast<uint8_t>(value);
        break;
      }

      case ChannelDataType::SignedIntegerLe:
      case ChannelDataType::SignedIntegerBe: {
        int64_t value = 0;
        valid = GetSignedValue(record_buffer, value);
        dest.resize(1);
        dest[0] = static_cast<uint8_t>(value);
        break;
      }

      case ChannelDataType::FloatLe:
      case ChannelDataType::FloatBe: {
        double value = 0;
        valid = GetFloatValue(record_buffer, value);
        dest.resize(1);
        dest[0] = static_cast<uint8_t>(value);
        break;
      }

      case ChannelDataType::StringUTF16Le:
      case ChannelDataType::StringUTF16Be:
      case ChannelDataType::StringUTF8:
      case ChannelDataType::StringAscii: {
        std::string text;
        valid = GetTextValue(record_buffer, text);
        dest.resize(text.size());
        memcpy(dest.data(), text.data(), text.size());
        break;
      }

      case ChannelDataType::MimeStream:
      case ChannelDataType::MimeSample:
      case ChannelDataType::ByteArray: {
        valid = GetByteArrayValue(record_buffer, dest);
        break;
      }

      case ChannelDataType::CanOpenDate: {
        uint64_t ms_since_1970 = 0;
        valid = GetCanOpenDate(record_buffer, ms_since_1970);
        dest.resize(1);
        dest[0] = static_cast<uint8_t>(ms_since_1970);
        break;
      }

      case ChannelDataType::CanOpenTime: {
        uint64_t ms_since_1970 = 0;
        valid = GetCanOpenTime(record_buffer, ms_since_1970);
        dest.resize(1);
        dest[0] = static_cast<uint8_t>(ms_since_1970);
        break;
      }

      default: break;
    }
    return valid;
  };

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
            auto date_array = util::time::NsToCanOpenDateArray(static_cast<uint64_t>(value));
            SetByteArray(date_array,valid);
          } else if (DataBytes() == 6) {
            auto time_array = util::time::NsToCanOpenTimeArray(static_cast<uint64_t>(value));
            SetByteArray(time_array,valid);
          }
        } else {
          SetValid(false);
        }
        break;

      case ChannelDataType::CanOpenDate:
        if (typeid(T) == typeid(uint64_t) && DataBytes() == 7) {
          const auto date_array = util::time::NsToCanOpenDateArray(static_cast<uint64_t>(value));
          SetByteArray(date_array,valid);
        } else {
          SetValid(false);
        }
        break;

      case ChannelDataType::CanOpenTime:
        if (typeid(T) == typeid(uint64_t) && DataBytes() == 6) {
          const auto time_array = util::time::NsToCanOpenTimeArray(static_cast<uint64_t>(value));
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

}
