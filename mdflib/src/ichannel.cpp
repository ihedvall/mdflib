

#include "mdf/ichannel.h"

#include <ctime>
#include <string>

#include "bigbuffer.h"
#include "half.hpp"
#include "littlebuffer.h"

namespace {

size_t RoundSizeToNearestPowerOfTwo(const size_t size) {
  size_t result = 1;
  while (result < size) {
    result <<= 1;
  }
  return result;
}

std::vector<uint8_t> ExtendIntegerBuffer(const std::vector<uint8_t> &data_buffer, const bool is_big_endian, const bool is_signed) {
  // Calculate the size of the extended buffer.
  const size_t rounded_size = RoundSizeToNearestPowerOfTwo(data_buffer.size());
  if (rounded_size == data_buffer.size()) {
    return data_buffer;
  }

  // Check if the number is negative.
  const uint8_t msb = (is_big_endian ? data_buffer.front() : data_buffer.back()) & 0x80;
  const bool is_negative = is_signed && (msb != 0);

  // Initialize the extended buffer with the sign bit.
  const uint8_t fill_byte = is_negative ? 0xFF : 0x00;
  std::vector<uint8_t> extended_buffer(rounded_size, fill_byte);

  // Copy the original data.
  if (is_big_endian) {
    memcpy(extended_buffer.data() + (rounded_size - data_buffer.size()), data_buffer.data(), data_buffer.size());
  } else {
    memcpy(extended_buffer.data(), data_buffer.data(), data_buffer.size());
  }

  return extended_buffer;
}

uint64_t ConvertUnsignedLe(const std::vector<uint8_t> &data_buffer) {
  const std::vector<uint8_t> extended_buffer = ExtendIntegerBuffer(data_buffer, false, false);
  switch (extended_buffer.size()) {
    case 1: {
      const mdf::LittleBuffer<uint8_t> data(extended_buffer, 0);
      return data.value();
    }

    case 2: {
      const mdf::LittleBuffer<uint16_t> data(extended_buffer, 0);
      return data.value();
    }

    case 4: {
      const mdf::LittleBuffer<uint32_t> data(extended_buffer, 0);
      return data.value();
    }

    case 8: {
      const mdf::LittleBuffer<uint64_t> data(extended_buffer, 0);
      return data.value();
    }

    default:
      break;
  }
  return 0;
}

uint64_t ConvertUnsignedBe(const std::vector<uint8_t> &data_buffer) {
  const std::vector<uint8_t> extended_buffer = ExtendIntegerBuffer(data_buffer, true, false);
  switch (extended_buffer.size()) {
    case 1: {
      const mdf::BigBuffer<uint8_t> data(extended_buffer, 0);
      return data.value();
    }

    case 2: {
      const mdf::BigBuffer<uint16_t> data(extended_buffer, 0);
      return data.value();
    }

    case 4: {
      const mdf::BigBuffer<uint32_t> data(extended_buffer, 0);
      return data.value();
    }

    case 8: {
      const mdf::BigBuffer<uint64_t> data(extended_buffer, 0);
      return data.value();
    }

    default:
      break;
  }
  return 0;
}

int64_t ConvertSignedLe(const std::vector<uint8_t> &data_buffer) {
  const std::vector<uint8_t> extended_buffer = ExtendIntegerBuffer(data_buffer, false, true);
  switch (extended_buffer.size()) {
    case 1: {
      const mdf::LittleBuffer<int8_t> data(extended_buffer, 0);
      return data.value();
    }

    case 2: {
      const mdf::LittleBuffer<int16_t> data(extended_buffer, 0);
      return data.value();
    }

    case 4: {
      const mdf::LittleBuffer<int32_t> data(extended_buffer, 0);
      return data.value();
    }

    case 8: {
      const mdf::LittleBuffer<int64_t> data(extended_buffer, 0);
      return data.value();
    }

    default:
      break;
  }
  return 0;
}

int64_t ConvertSignedBe(const std::vector<uint8_t> &data_buffer) {
  const std::vector<uint8_t> extended_buffer = ExtendIntegerBuffer(data_buffer, true, true);
  switch (extended_buffer.size()) {
    case 1: {
      const mdf::BigBuffer<int8_t> data(extended_buffer, 0);
      return data.value();
    }

    case 2: {
      const mdf::BigBuffer<int16_t> data(extended_buffer, 0);
      return data.value();
    }

    case 4: {
      const mdf::BigBuffer<int32_t> data(extended_buffer, 0);
      return data.value();
    }

    case 8: {
      const mdf::BigBuffer<int64_t> data(extended_buffer, 0);
      return data.value();
    }

    default:
      break;
  }
  return 0;
}

double ConvertFloatBe(const std::vector<uint8_t> &data_buffer) {
  switch (data_buffer.size()) {
    case 2: {
      throw std::logic_error("Half-precision float is not implemented yet.");
      /*

            const mdf::BigBuffer<half_float::half> data(data_buffer, 0);
            return data.value();
          }
      */
    }
    case 4: {
      const mdf::BigBuffer<float> data(data_buffer, 0);
      return data.value();
    }

    case 8: {
      const mdf::BigBuffer<double> data(data_buffer, 0);
      return data.value();
    }
    default:
      break;
  }
  throw std::invalid_argument("Invalid float size. Float must be 2, 4 or 8 bytes.");
}

double ConvertFloatLe(const std::vector<uint8_t> &data_buffer) {
  switch (data_buffer.size()) {
    case 2: {
      throw std::logic_error("Half-precision float is not implemented yet.");
      /*
            boost::endian::endian_buffer<boost::endian::order::little,
         half_float::half, 16, boost::endian::align::no> data {};
            memcpy(data.data(), data_buffer.data(), 2);
            return data.value();
      */
    }
    case 4: {
      const mdf::LittleBuffer<float> data(data_buffer, 0);
      return data.value();
    }

    case 8: {
      const mdf::LittleBuffer<double> data(data_buffer, 0);
      return data.value();
    }

    default:
      break;
  }
  throw std::invalid_argument("Invalid float size. Float must be 2, 4 or 8 bytes.");
}

}  // namespace
namespace mdf {

void IChannel::CopyToDataBuffer(const std::vector<uint8_t> &record_buffer,
                                std::vector<uint8_t> &data_buffer) const {
  size_t nof_bytes = (BitCount() / 8) + (BitCount() % 8 > 0 ? 1 : 0);
  if (data_buffer.size() != nof_bytes) {
    data_buffer.resize(nof_bytes);
  }
  memset(data_buffer.data(), 0, data_buffer.size());

  if (BitOffset() == 0 && (BitCount() % 8) == 0) {
    // This is the preferred way of doing business
    memcpy(data_buffer.data(), record_buffer.data() + ByteOffset(),
           data_buffer.size());
  } else if (BitCount() == 1) {
    // This is OK for boolean and bits
    uint8_t in_mask = 0x01 << BitOffset();
    data_buffer[0] = (record_buffer[ByteOffset()] & in_mask) != 0
                         ? 0x01 : 0x00;
  } else {
    // Need to bit mask copy everything.
    const auto first_byte = ByteOffset();
    const auto last_byte = first_byte + nof_bytes - 1;
    const bool is_big_endian = ChannelDataType() == ChannelDataType::SignedIntegerBe ||
                               ChannelDataType() == ChannelDataType::UnsignedIntegerBe;

    // Combine bytes
    uint64_t value = 0;
    if (is_big_endian) {
      for (auto i = first_byte; i <= last_byte; ++i) {
        value <<= 8;
        value |= record_buffer[i];
      }
    } else {
      for (auto i = last_byte; i >= first_byte; --i) {
        value <<= 8;
        value |= record_buffer[i];
      }
    }

    // Apply offset
    value >>= BitOffset();

    // Apply mask
    const uint64_t bit_limit_mask = (1ULL << BitCount()) - 1;
    value &= bit_limit_mask;

    // Apply sign extension
    if (DataType() == ChannelDataType::SignedIntegerLe ||
                            DataType() == ChannelDataType::SignedIntegerBe) {
      const bool is_negative = (value & (0x01ULL << (BitCount() - 1))) != 0;
      if (is_negative) {
        const uint64_t sign_extension_mask = ~((1ULL << BitCount()) - 1);
        value |= sign_extension_mask;
      }
    }

    // Copy to output buffer
    if (is_big_endian) {
      for (auto i = 0; i < nof_bytes; ++i) {
        data_buffer[nof_bytes - i - 1] = static_cast<uint8_t>(value & 0xFF);
        value >>= 8;
      }
    } else {
      for (auto i = 0; i < nof_bytes; ++i) {
        data_buffer[i] = static_cast<uint8_t>(value & 0xFF);
        value >>= 8;
      }
    }
  }

}

bool IChannel::GetUnsignedValue(const std::vector<uint8_t> &record_buffer,
                                uint64_t &dest) const {
  // Copy to a temp data buffer, so we can get rid of this bit offset nonsense

  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
  if (Type() == ChannelType::VariableLength) {
    dest = ConvertUnsignedLe(data_buffer);
    return true;
  }

  switch (DataType()) {
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::UnsignedIntegerLe:
      dest = ConvertUnsignedLe(data_buffer);
      break;

    case ChannelDataType::UnsignedIntegerBe:
      dest = ConvertUnsignedBe(data_buffer);
      break;

    default:
      return false;  // Not valid conversion
  }
  return true;
}

bool IChannel::GetSignedValue(const std::vector<uint8_t> &record_buffer,
                              int64_t &dest) const {
  // Copy to a temp data buffer, so we can get rid of the bit offset nonsense
  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
  switch (DataType()) {
    case ChannelDataType::SignedIntegerLe:
      dest = ConvertSignedLe(data_buffer);
      break;

    case ChannelDataType::SignedIntegerBe:
      dest = ConvertSignedBe(data_buffer);
      break;

    default:
      return false;  // Not valid conversion
  }
  return true;
}

bool IChannel::GetFloatValue(const std::vector<uint8_t> &record_buffer,
                             double &dest) const {
  // Copy to a temp data buffer, so we can get rid of the bit offset nonsense
  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
  switch (DataType()) {
    case ChannelDataType::FloatLe:
      dest = ConvertFloatLe(data_buffer);
      break;

    case ChannelDataType::FloatBe:
      dest = ConvertFloatBe(data_buffer);
      break;

    default:
      return false;  // Not valid conversion
  }
  return true;
}

bool IChannel::GetTextValue(const std::vector<uint8_t> &record_buffer,
                            std::string &dest) const {
  auto offset = ByteOffset();
  size_t nof_bytes = BitCount() / 8 + ((BitCount() % 8) != 0 ? 1 : 0);



  bool valid = true;
  dest.clear();

  switch (DataType()) {
    case ChannelDataType::StringAscii: {
      // Convert ASCII to UTF8
      std::ostringstream s;
      for (size_t ii = 0;
           ii < nof_bytes && ii + offset < record_buffer.size();
           ++ii) {
        char in = static_cast<char>(record_buffer[ii + offset]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      try {
        dest = MdfHelper::Latin1ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
        dest = s.str();
      }
      break;
    }

    case ChannelDataType::StringUTF8: {
      // No conversion needed
      std::ostringstream s;
      for (size_t ii = offset; ii < record_buffer.size(); ++ii) {
        char in = static_cast<char>(record_buffer[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      dest = s.str();
      break;
    }

    case ChannelDataType::StringUTF16Le: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        const LittleBuffer<uint16_t> data(record_buffer, ii);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = MdfHelper::Utf16ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
      }
      break;
    }

    case ChannelDataType::StringUTF16Be: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        const BigBuffer<uint16_t> data(record_buffer, ii);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = MdfHelper::Utf16ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
      }
      break;
    }
    default:
      break;
  }
  return valid;
}

bool IChannel::GetByteArrayValue(const std::vector<uint8_t> &record_buffer,
                                 std::vector<uint8_t> &dest) const {

  if (Type() == ChannelType::VariableLength && VlsdRecordId() > 0) {
    dest = record_buffer;
  } else {
    if (dest.size() != DataBytes()) {
      dest.resize(DataBytes());
    }
    if (dest.empty()) {
      return true;
    }
    memcpy(dest.data(), record_buffer.data() + ByteOffset(), DataBytes());
  }
  return true;
}

bool IChannel::GetCanOpenDate(const std::vector<uint8_t> &record_buffer,
                              uint64_t &dest) const {
  std::vector<uint8_t> date_array(7, 0);
  memcpy(date_array.data(), record_buffer.data() + ByteOffset(),
         date_array.size());
  dest = MdfHelper::CanOpenDateArrayToNs(date_array);
  return true;
}

bool IChannel::GetCanOpenTime(const std::vector<uint8_t> &record_buffer,
                              uint64_t &dest) const {
  std::vector<uint8_t> time_array(6, 0);
  memcpy(time_array.data(), record_buffer.data() + ByteOffset(),
         time_array.size());
  dest = MdfHelper::CanOpenTimeArrayToNs(time_array);
  return true;
}

void IChannel::SetValid(bool) {
  // Only MDF 4 have this functionality;
}
bool IChannel::GetValid(const std::vector<uint8_t> &) const {
  return true; // Only MDF 4 have this functionality;
}

void IChannel::SetUnsignedValueLe(uint64_t value, bool valid) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    return; // Invalid use of function
  }

  const size_t bytes = BitCount() / 8;
  const auto max_bytes = bytes + ByteOffset();
  if (max_bytes > buffer.size()) {
    SetValid(false);
    return;
  }

  SetValid(valid);
  switch (bytes) {
    case 1: {
      const LittleBuffer data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      const LittleBuffer data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      const LittleBuffer data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      const LittleBuffer data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetUnsignedValueBe(uint64_t value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      const BigBuffer data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      const BigBuffer data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      const BigBuffer data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      const BigBuffer data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }
    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetSignedValueLe(int64_t value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      const LittleBuffer data(static_cast<int8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      const LittleBuffer data(static_cast<int16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      const LittleBuffer data(static_cast<int32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      const LittleBuffer data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetSignedValueBe(int64_t value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      const BigBuffer data(static_cast<int8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      const BigBuffer data(static_cast<int16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      const BigBuffer data(static_cast<int32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      const BigBuffer data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }
    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetFloatValueLe(double value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 4: {
      const LittleBuffer data(static_cast<float>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      const LittleBuffer data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetFloatValueBe(double value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 4: {
      const BigBuffer data(static_cast<float>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      const BigBuffer data(value);
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    default:
      SetValid(false);
      break;
  }
}

void IChannel::SetTextValue(const std::string &value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() < bytes) {
    memcpy(buffer.data() + ByteOffset(), value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(), value.data(), bytes - 1);
  }
}

void IChannel::SetByteArray(const std::vector<uint8_t> &value, bool valid) {
  SetValid(valid);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() <= bytes) {
    memcpy(buffer.data() + ByteOffset(), value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(), value.data(), bytes);
  }
}

template <>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               std::vector<uint8_t> &dest) const {
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

    default:
      break;
  }
  if (valid) {
    valid = GetValid(record_buffer);
  }
  return valid;
}

template <>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               std::string &dest) const {
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
      dest = IsDecimalUsed() ? MdfHelper::FormatDouble(value, Decimals())
                             : std::to_string(value);
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
      for (const auto byte : list) {
        s << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
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
      const struct tm *bt = std::localtime(&time);

      std::ostringstream text;
      text << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0')
           << std::setw(3) << ms;
      dest = text.str();
      break;
    }

    case ChannelDataType::CanOpenTime: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenTime(record_buffer, ms_since_1970);

      const auto ms = ms_since_1970 % 1000;
      const auto time = static_cast<time_t>(ms_since_1970 / 1000);
      const struct tm *bt = std::localtime(&time);

      std::ostringstream text;
      text << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0')
           << std::setw(3) << ms;
      dest = text.str();
      break;
    }

    default:
      break;
  }
  if (valid) {
    valid = GetValid(record_buffer);
  }
  return valid;
}

template <>
void IChannel::SetChannelValue(const std::string &value, bool valid) {
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
      SetUnsignedValueLe(std::stoull(value), valid);
      break;

    case ChannelDataType::UnsignedIntegerBe:
      SetUnsignedValueBe(std::stoull(value), valid);
      break;

    case ChannelDataType::SignedIntegerLe:
      SetSignedValueLe(std::stoll(value), valid);
      break;

    case ChannelDataType::SignedIntegerBe:
      SetSignedValueBe(std::stoll(value), valid);
      break;

    case ChannelDataType::FloatLe:
      SetFloatValueLe(std::stod(value), valid);
      break;

    case ChannelDataType::FloatBe:
      SetFloatValueBe(std::stod(value), valid);
      break;

    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
      SetTextValue(value, valid);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:      {
        // The input string needs to be copied to a byte array.
        auto byte_array = MdfHelper::TextToByteArray(value);
        SetByteArray(byte_array, valid);
        break;
      }

    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime:
    default:
        // Cannot find any good text to date/time
      SetValid(false);
      break;
  }
}

template <>
void IChannel::SetChannelValue(const std::vector<uint8_t> &value, bool valid) {
  switch (DataType()) {
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      SetByteArray(value, valid);
      break;

    case ChannelDataType::CanOpenTime:
    case ChannelDataType::CanOpenDate:
      if (value.size() == DataBytes()) {
        SetByteArray(value, valid);
      } else {
        SetValid(false);
      }
      break;

    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
    default:
      // Conversion to byte array is suspicious
      SetValid(false);
      break;
  }
}

void IChannel::Sync(ChannelSyncType type) {}

ChannelSyncType IChannel::Sync() const { return ChannelSyncType::None; }

void IChannel::Range(double min, double max) {}

std::optional<std::pair<double, double>> IChannel::Range() const { return {}; }

void IChannel::Limit(double min, double max) {}

std::optional<std::pair<double, double>> IChannel::Limit() const { return {}; }

void IChannel::ExtLimit(double min, double max) {}

std::optional<std::pair<double, double>> IChannel::ExtLimit() const {
  return {};
}
IMetaData *IChannel::CreateMetaData() { return nullptr; }
IMetaData *IChannel::MetaData() const { return nullptr; }

ISourceInformation *IChannel::SourceInformation() const {
  // Only supported by MDF4
  return nullptr;
}
ISourceInformation *IChannel::CreateSourceInformation() {
  // Only supported by MDF4
  return nullptr;
}

void IChannel::Flags(uint32_t flags) {}
uint32_t IChannel::Flags() const { return 0; }

void IChannel::SetTimestamp(double timestamp,
                            std::vector<uint8_t> &record_buffer) const {
  // If conversion is in use, reverse convert to channel value
  const auto* conversion = ChannelConversion();
  if (conversion != nullptr &&
      conversion->Type() == ConversionType::Linear &&
      conversion->Parameter(1) != 0.0) {
    timestamp -= conversion->Parameter(0);
    timestamp /= conversion->Parameter(1);
  } else if (conversion != nullptr) {
    return;
  }

  const size_t bytes = BitCount() / 8;

  switch (DataType()) {
    case ChannelDataType::FloatLe:
      if (bytes == 4) {
        const LittleBuffer data(static_cast<float>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const LittleBuffer data(static_cast<double>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    case ChannelDataType::FloatBe:
      if (bytes == 4) {
        const BigBuffer data(static_cast<float>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const BigBuffer data(static_cast<double>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    default:
      break;
  }
}

IChannel *IChannel::CreateChannelComposition(const std::string_view &name) {
  auto list = ChannelCompositions();
  auto itr = std::find_if(list.begin(), list.end(), [&] (auto* channel) {
    return channel != nullptr && channel->Name() == name;
  });
  if (itr != list.end()) {
    return *itr;
  }
  auto* new_channel = CreateChannelComposition();
  if (new_channel != nullptr) {
    new_channel->Name(name.data());
  }
  return new_channel;
}

}  // end namespace mdf
