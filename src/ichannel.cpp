/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <string>
#define BOOST_NO_AUTO_PTR
#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/locale.hpp>


#include "mdf/ichannel.h"
#include "iblock.h"
#include "datablock.h"
#include "datalistblock.h"

namespace {

uint64_t ConvertUnsignedLe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::little_uint8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::little_uint16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::little_uint24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::little_uint32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::little_uint40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::little_uint48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::little_uint56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::little_uint64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

uint64_t ConvertUnsignedBe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::big_uint8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::big_uint16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::big_uint24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::big_uint32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::big_uint40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::big_uint48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::big_uint56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::big_uint64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

int64_t ConvertSignedLe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::little_int8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::little_int16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::little_int24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::little_int32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::little_int40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::little_int48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::little_int56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::little_int64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

int64_t ConvertSignedBe(const std::vector<uint8_t>& data_buffer)
{
  switch (data_buffer.size()) {
    case 1: {
      boost::endian::big_int8_buf_t data;
      memcpy(data.data(), data_buffer.data(), 1);
      return data.value();
    }

    case 2: {
      boost::endian::big_int16_buf_t data;
      memcpy(data.data(), data_buffer.data(), 2);
      return data.value();
    }

    case 3: {
      boost::endian::big_int24_buf_t data;
      memcpy(data.data(), data_buffer.data(), 3);
      return data.value();
    }

    case 4: {
      boost::endian::big_int32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 5: {
      boost::endian::big_int40_buf_t data;
      memcpy(data.data(), data_buffer.data(), 5);
      return data.value();
    }

    case 6: {
      boost::endian::big_int48_buf_t data;
      memcpy(data.data(), data_buffer.data(), 6);
      return data.value();
    }

    case 7: {
      boost::endian::big_int56_buf_t data;
      memcpy(data.data(), data_buffer.data(), 7);
      return data.value();
    }

    case 8: {
      boost::endian::big_int64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }

    default:break;
  }
  return 0;
}

double ConvertFloatBe(const std::vector<uint8_t>& data_buffer) {
  switch (data_buffer.size()) {
    case 4: {
      boost::endian::big_float32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 8: {
      boost::endian::big_float64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }
    default: break;
  }
  return 0.0;
}

double ConvertFloatLe(const std::vector<uint8_t>& data_buffer) {
  switch (data_buffer.size()) {
    case 4: {
      boost::endian::little_float32_buf_t data;
      memcpy(data.data(), data_buffer.data(), 4);
      return data.value();
    }

    case 8: {
      boost::endian::little_float64_buf_t data;
      memcpy(data.data(), data_buffer.data(), 8);
      return data.value();
    }
    default: break;
  }
  return 0.0;
}


} // namespace
namespace mdf {

void IChannel::CopyToDataBuffer(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t>& data_buffer) const {
  size_t nof_bytes = (BitCount() / 8) + (BitCount() % 8 > 0 ? 1 : 0);
  if (data_buffer.size() != nof_bytes) {
    data_buffer.resize(nof_bytes);
  }
  memset(data_buffer.data(), 0, data_buffer.size());

  if (BitOffset() == 0) {
    // This is the preferred way of doing business
    memcpy(data_buffer.data(),record_buffer.data() + ByteOffset(), data_buffer.size());
  } else if (BitCount() == 1) {
    // This is OK for boolean and bits
    uint8_t in_bit = 0x01 << (BitOffset() - 1);
    data_buffer[0] = (record_buffer[ByteOffset()] & in_bit)? 0x01 : 0x00;
  } else {
    uint8_t in_offset = BitOffset();
    uint32_t in_byte = ByteOffset();
    for (auto ii = BitCount(); ii < BitCount(); ++ii) {
      uint8_t in_bit = 0x01 << (in_offset - 1);
      auto out_byte = ii / 8;
      if (record_buffer[in_byte] & in_bit) {
        data_buffer[out_byte] |= 0x01;
      }
      if (ii < (BitCount() - 1)) {
        data_buffer[out_byte] <<= 1;
      }

      --in_offset;
      if (in_offset == 0) {
        in_offset = 8;
        ++in_byte;
      }
    }

  }
}

bool IChannel::GetUnsignedValue(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const {

  // Copy to a temp data buffer, so we can get rid of this bit offset nonsense
  std::vector<uint8_t> data_buffer;
  CopyToDataBuffer(record_buffer, data_buffer);
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
      return false; // Not valid conversion
  }
  return true;
}

bool IChannel::GetSignedValue(const std::vector<uint8_t> &record_buffer, int64_t &dest) const {

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
      return false; // Not valid conversion
  }
  return true;
}

bool IChannel::GetFloatValue(const std::vector<uint8_t> &record_buffer, double &dest) const {

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
      return false; // Not valid conversion
  }
  return true;
}

bool IChannel::GetTextValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const {
  auto offset = ByteOffset();
  bool valid = true;
  dest.clear();

  switch (DataType()) {
    case ChannelDataType::StringAscii: {
      // Convert ASCII to UTF8
      std::ostringstream s;
      for (size_t ii = offset; ii < record_buffer.size(); ++ii) {
        char in = static_cast<char>(record_buffer[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      try {
        dest = boost::locale::conv::to_utf<char>(s.str(), "Latin1");
      } catch (const std::exception&) {
        valid = false; // Conversion error
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
        auto* d = record_buffer.data() + ii;
        boost::endian::little_uint16_buf_t data;
        memcpy(data.data(), record_buffer.data() + ii, 2);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = boost::locale::conv::utf_to_utf<char>(s.str());
      } catch (const std::exception&) {
        valid = false; // Conversion error
      }
      break;
    }

    case ChannelDataType::StringUTF16Be: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        auto* d = record_buffer.data() + ii;
        boost::endian::big_uint16_buf_t data;
        memcpy(data.data(), d, 2);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = boost::locale::conv::utf_to_utf<char>(s.str());
      } catch (const std::exception&) {
        valid = false; // Conversion error
      }
      break;
    }
    default:
      break;
  }
  return valid;
}

bool IChannel::GetByteArrayValue(const std::vector<uint8_t> &record_buffer, std::vector<uint8_t> &dest) const {
  if (dest.size() != DataBytes()) {
    dest.resize(DataBytes());
  }
  if (dest.empty()) {
    return true;
  }
  memcpy(dest.data(),record_buffer.data() + ByteOffset(), DataBytes());
  return true;
}

bool IChannel::GetCanOpenDate(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const {
  std::vector<uint8_t> date_array(7,0);
  memcpy(date_array.data(), record_buffer.data() + ByteOffset(), date_array.size());
  dest = util::time::CanOpenDateArrayToNs(date_array);
  return true;
}

bool IChannel::GetCanOpenTime(const std::vector<uint8_t> &record_buffer, uint64_t &dest) const {
  std::vector<uint8_t> time_array(6,0);
  memcpy(time_array.data(), record_buffer.data() + ByteOffset(), time_array.size());
  dest = util::time::CanOpenTimeArrayToNs(time_array);
  return true;
}

void IChannel::SetValid(bool) {
  // Only MDF4 have this functionality
}

void IChannel::SetUnsignedValueLe(uint64_t value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::little_uint8_buf_at data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::little_uint16_buf_at data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::little_uint32_buf_at data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::little_uint64_buf_at data(value);
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
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::big_uint8_buf_at data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::big_uint16_buf_at data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::big_uint32_buf_at data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::big_uint64_buf_at data(value);
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
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::little_int8_buf_at data(static_cast<int8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::little_int16_buf_at data(static_cast<int16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::little_int32_buf_at data(static_cast<int32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::little_int64_buf_at data(value);
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
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 1: {
      boost::endian::big_int8_buf_at data(static_cast<int8_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 2: {
      boost::endian::big_int16_buf_at data(static_cast<int16_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 4: {
      boost::endian::big_int32_buf_at data(static_cast<int32_t>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::big_int64_buf_at data(value);
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
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 4: {
      boost::endian::little_float32_buf_at data(static_cast<float>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::little_float64_buf_at data(value);
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
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  switch (bytes) {
    case 4: {
      boost::endian::big_float32_buf_at data(static_cast<float>(value));
      memcpy(buffer.data() + ByteOffset(), data.data(), bytes);
      break;
    }

    case 8: {
      boost::endian::big_float64_buf_at data(value);
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
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() < bytes) {
    memcpy(buffer.data() + ByteOffset(),value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(),value.data(), bytes -1);
  }
}

void IChannel::SetByteArray(const std::vector<uint8_t> &value, bool valid) {
  SetValid(valid);
  auto& buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() <= bytes) {
    memcpy(buffer.data() + ByteOffset(),value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(),value.data(), bytes);
  }
}

template<>
void IChannel::SetChannelValue(const std::string& value, bool valid) {
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
      SetTextValue(value, valid);
      break;

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
    default:
      SetValid(false);
      break;
  }
}

template<>
void IChannel::SetChannelValue(const std::vector<uint8_t>& value, bool valid) {
  switch (DataType()) {
    case ChannelDataType::ByteArray:
      SetByteArray(value, valid);
      break;

    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    default:
      SetValid(false);
      break;
  }
}



} // end namespace mdf