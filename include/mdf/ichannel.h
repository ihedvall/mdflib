/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannel.h
 * \brief The define an interface against a channel block (CN).
 */
#pragma once
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "mdf/iblock.h"
#include "mdf/ichannelconversion.h"
#include "mdf/imetadata.h"
#include "mdf/isourceinformation.h"
#include "mdf/mdfhelper.h"

namespace mdf {

/** \brief Channel functional type.
 *
 * Most channels are marked as 'FixedLength' which means that its
 * size in a record is fixed. This works well with most data types but
 * byte arrays and strings may change its size. Instead are these data types
 * marked as 'Variable Length'. Avoid writing variable length data as it
 * may allocate a lot of memory as it flush at the end of the measurement.
 *
 * One channel in channel group (IChannelGroup), should be marked as a master
 * channel. This channel is typical relative sample time with seconds as
 * unit. The master channel is typical used on the X-axis when plotting data.
 *
 * The 'VirtualMaster' channel can be used if the sample number is linear
 * related to the sample time. The channel conversion (CC) block should
 * define the sample number to time conversion.
 *
 * The 'Sync' channel is used to synchronize an attachment block (file).
 *
 * The 'MaxLength' type is typical used when storing CAN byte array where
 * another channel stores actual bytes stored in a sample. For CAN the size
 * in the max record size is 8 bytes.
 *
 * The 'VirtualData' is similar to the 'VirtualMaster' channel but related to
 * data. Good luck to find a use of this type.
 */
enum class ChannelType : uint8_t {
  FixedLength = 0,    ///< Fixed length data (default type)
  VariableLength = 1, ///< Variable length data
  Master = 2,         ///< Master channel
  VirtualMaster = 3,  ///< Virtual master channel
  Sync = 4,           ///< Synchronize channel
  MaxLength = 5,      ///< Max length channel
  VirtualData = 6     ///< Virtual data channel
};

/** \brief Synchronization type
 *
 * Defines the synchronization type. The type is 'None' for fixed length
 * channel but should be set for master and synchronization channels.
 */
enum class ChannelSyncType : uint8_t {
  None = 0,     ///< No synchronization (default value)
  Time = 1,     ///< Time type
  Angle = 2,    ///< Angle type
  Distance = 3, ///< Distance type
  Index = 4     ///< Sample number
};

/** \brief Channel data type.
 *
 * Defines the channel data type. Avoid defining value sizes that doesn't align
 * to a byte size.
 *
 * The Le and Be extension is related to byte order. Little endian (Intel
 * byte order) while big endian (Motorola byte order).
 */
enum class ChannelDataType : uint8_t {
  UnsignedIntegerLe= 0,  ///< Unsigned integer, little endian.
  UnsignedIntegerBe = 1, ///< Unsigned integer, big endian.
  SignedIntegerLe = 2,   ///< Signed integer, little endian.
  SignedIntegerBe = 3,   ///< Signed integer, big endian.
  FloatLe = 4,           ///< Float, little endian.
  FloatBe = 5,           ///< Float, big endian.
  StringAscii = 6,       ///< Text,  ISO-8859-1 coded
  StringUTF8 = 7,        ///< Text, UTF8 coded.
  StringUTF16Le = 8,     ///< Text, UTF16 coded little endian.
  StringUTF16Be = 9,     ///< Text, UTF16 coded big endian.
  ByteArray = 10,        ///< Byte array.
  MimeSample = 11,       ///< MIME sample byte array.
  MimeStream = 12,       ///< MIME stream byte array.
  CanOpenDate = 13,      ///< 7-byte CANOpen date.
  CanOpenTime = 14,      ///< 6-byte CANOpen time.
  ComplexLe = 15,        ///< Complex value, little endian.
  ComplexBe = 16         ///< Complex value, big endian.
};

/** \brief Channel flags. See also IChannel::Flags().
 *
 */
namespace CnFlag {
constexpr uint32_t AllValuesInvalid = 0x0001; ///< All values are invalid.
constexpr uint32_t InvalidValid = 0x0002; ///< Invalid bit is used.
constexpr uint32_t PrecisionValid = 0x0004; ///< Precision is used.
constexpr uint32_t RangeValid = 0x0008; ///< Range is used.
constexpr uint32_t LimitValid = 0x0010; ///< Limit is used.
constexpr uint32_t ExtendedLimitValid = 0x0020; ///< Extended limit is used.
constexpr uint32_t Discrete = 0x0040; ///< Discrete channel.
constexpr uint32_t Calibration = 0x0080; ///< Calibrated channel.
constexpr uint32_t Calculated = 0x0100; ///< Calculated channel.
constexpr uint32_t Virtual = 0x0200; ///< Virtual channel.
constexpr uint32_t BusEvent = 0x0400; ///< Bus event channel.
constexpr uint32_t StrictlyMonotonous = 0x0800; ///< Strict monotonously.
constexpr uint32_t DefaultX = 0x1000; ///< Default x-axis channel.
constexpr uint32_t EventSignal = 0x2000; ///< Event signal.
constexpr uint32_t VlsdDataStream = 0x4000; ///< VLSD data stream channel.
}  // namespace CnFlag

/** \brief Defines a MDF channel (CN) block.
 *
 */
class IChannel : public IBlock  {
 public:

  virtual void Name(const std::string &name) = 0; ///< Sets channel name
  [[nodiscard]] virtual std::string Name() const = 0; ///< Returns channel name

  virtual void DisplayName(const std::string &name) = 0; ///< Sets display name.
  [[nodiscard]] virtual std::string DisplayName() const = 0; ///< Display name.

  /** \brief Sets the description. */
  virtual void Description(const std::string &description) = 0;

  /** \brief Returns the description. */
  [[nodiscard]] virtual std::string Description() const = 0;

  /** \brief Sets unit string or or the MIME text string.
   *
   * The function sets the unit string for the channel. If the channel data
   * type is a MIME sample or a stream, the unit is a mime content type string.
   */
  virtual void Unit(const std::string &unit) = 0;

  /** \brief Returns the unit string or the MIME content type string.
   *
   *
   * @return Unit or MIME content string.
   */
  [[nodiscard]] virtual std::string Unit() const = 0; ///< Returns the unit

  /** \brief Sets channel flags. Flags are defined in the CnFlag namespace  */
  virtual void Flags(uint32_t flags);

  /** \brief Channel flags are defined in the CnFlag namespace  */
  [[nodiscard]] virtual uint32_t Flags() const;

  [[nodiscard]] virtual bool IsUnitValid() const = 0; ///< True if unit exists.

  virtual void Type(ChannelType type) = 0; ///< Sets the type of channel.
  [[nodiscard]] virtual ChannelType Type() const = 0; ///< Type of channel.

  virtual void Sync(ChannelSyncType type); ///< Sets the type of sync.
  [[nodiscard]] virtual ChannelSyncType Sync() const; ///< Type of sync.

  virtual void DataType(ChannelDataType type) = 0; ///< Sets the data type.
  [[nodiscard]] virtual ChannelDataType DataType() const = 0; ///< Data type.

  virtual void DataBytes(uint64_t nof_bytes) = 0; ///< Sets the data size (bytes)
  [[nodiscard]] virtual uint64_t DataBytes() const = 0; ///< Data size (bytes);

  /** \brief Number of decimals (floating points)  */
  [[nodiscard]] virtual uint8_t Decimals() const = 0;

  /** \brief Returns true if decimals is used  */
  [[nodiscard]] virtual bool IsDecimalUsed() const = 0;

  /** \brief Sets the ranges.  */
  virtual void Range(double min, double max);

  /** \brief Returns the ranges.  */
  [[nodiscard]] virtual std::optional<std::pair<double, double>> Range() const;

  /** \brief Sets the limits.  */
  virtual void Limit(double min, double max);

  /** \brief Returns the limits.  */
  [[nodiscard]] virtual std::optional<std::pair<double, double>> Limit() const;

  /** \brief Sets the extended limits.  */
  virtual void ExtLimit(double min, double max);

  /** \brief Returns the extended limits.  */
  [[nodiscard]] virtual std::optional<std::pair<double, double>> ExtLimit()
      const;

  /** \brief Sets the sample rate (s). This is a MDF 3 feature.  */
  virtual void SamplingRate(double sampling_rate) = 0;

  /** \brief Returns the sample rate (s). This is a MDF 3 feature.  */
  [[nodiscard]] virtual double SamplingRate() const = 0;

  /** \brief Returns the source information, if any. */
  [[nodiscard]] virtual ISourceInformation *SourceInformation() const;

  /** \brief Creates a source information block. */
  [[nodiscard]] virtual ISourceInformation* CreateSourceInformation();

  /** \brief Returns the conversion block, if any. */
  [[nodiscard]] virtual IChannelConversion *ChannelConversion() const = 0;

  /** \brief Creates a conversion block. */
  [[nodiscard]] virtual IChannelConversion *CreateChannelConversion() = 0;

  /** \brief Creates a composition channel.
   *
   * Creates a composition channel block. Composition channels reference their
   * bit length and bit offset to its parent channel. In reality, the parent
   * channel must be a byte array.
   */
  [[nodiscard]] virtual IChannel *CreateChannelComposition() = 0;

  /** \brief Creates a composition channel with a specific name or returns an
   * existing channel.
   *
   * Creates a composition channel block. Composition channels reference their
   * bit length and bit offset to its parent channel. In reality, the parent
   * channel must be a byte array.
   */
  [[nodiscard]] virtual IChannel *CreateChannelComposition(
      const std::string_view& name);
  /** \brief Creates a composition channel.
   *
   * Creates a composition channel block. Composition channels reference their
   * bit length and bit offset to its parent channel. In reality, the parent
   * channel must be a byte array.
   */
  [[nodiscard]] virtual std::vector<IChannel*> ChannelCompositions() = 0;

  /** \brief Returns true if the channel is a number. */
  [[nodiscard]] bool IsNumber() const {
    // Need to check the cc at well if it is a value to text conversion
    if (const auto *cc = ChannelConversion();
        cc != nullptr && cc->Type() <= ConversionType::ValueRangeToValue) {
      return true;
    }
    return DataType() <= ChannelDataType::FloatBe;
  }

  /** \brief Creates a metadata (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData();

  /** \brief Returns the  meta-data (MD) block if it exist. */
  [[nodiscard]] virtual IMetaData* MetaData() const;

  /** \brief Sets the VLSD record id.
   *
   * This function is mainly used internally and solves the problem
   * if the channel is a variable length and its value is stored in
   * another (VLSD) channel group (CG). This property is set to the record ID
   * of the VLSD block.
   * @param record_id Record id of the block storing the data bytes.
   */
  void VlsdRecordId(uint64_t record_id) const {
    vlsd_record_id_ = record_id;
  }

  /** \brief Returns the VLSD record id.
   *
   * This function is mainly used internally and solves the problem
   * if the channel is a variable length and its value is stored in
   * another (VLSD) channel group (CG). This property is set to the record ID
   * of the VLSD block.
   * @return Record id of the block storing the data bytes.
   */
  [[nodiscard]] uint64_t VlsdRecordId() const {
    return vlsd_record_id_;
  }

  /** \brief Parse out the channel value from a data record.
   *
   * Internally used function that parse out the channel value from
   * a sample record.
   *
   * @tparam T Type of value to return
   * @param record_buffer Data record from a data block.
   * @param dest Destination value.
   * @return True if the value is valid.
   */
  template <typename T>
  bool GetChannelValue(const std::vector<uint8_t> &record_buffer,
                       T &dest) const;

  /** \brief Fills a record buffer with a channel value.
   *
   * Internally used function that fills a sample record buffer with a channel
   * value.
   * @tparam T Type of value.
   * @param value The value to transfer.
   * @param valid True if the value is valid.
   */
  template <typename T>
  void SetChannelValue(const T &value, bool valid = true);

  /** \brief Internally used function mainly for fetching VLSD index values.
   *
   * @param record_buffer The sample record buffer.
   * @param dest Destination value.
   * @return True if the value is valid.
   */
  virtual bool GetUnsignedValue(const std::vector<uint8_t> &record_buffer,
                                uint64_t &dest) const;
  /** \brief Internally used function mainly for fetching VLSD text values.
    *
    * @param record_buffer The sample record buffer.
    * @param dest Destination value.
    * @return True if the value is valid.
    */
  virtual bool GetTextValue(const std::vector<uint8_t> &record_buffer,
                            std::string &dest) const;
  /** \brief The function change the supplied records time channel value.
   *
   * The function update the record buffer with a new time. This function is
   * for internal use and its purpose is to change the timestamps in
   * the sample cache queue, from absolute time to relative time. This
   * happens when the MdfWriter::StartMeasurement() function is called
   * @param timestamp Relative time in seconds.
   * @param record_buffer The record buffer to update.
   */
  void SetTimestamp(double timestamp, std::vector<uint8_t> &record_buffer) const;

  /** \brief Sets the size of data in bits.
   *
   * This function shall only be used for MDF4 files and for composition
   * channels. Composition channels reference their parent byte array channel.
   * For ordinary channels, the bit size and offset is calculated when the
   * measurement is initialized.
   * @param bits Number of bits.
   */
  virtual void BitCount(uint32_t bits) = 0;

  /** \brief Returns the data size in number of bits */
  [[nodiscard]] virtual uint32_t BitCount() const = 0;

  /** \brief Sets the offset to data in bits.
   *
   * This function shall only be used for MDF4 files and for composition
   * channels. Composition channels reference their parent byte array channel.
   * For ordinary channels, the bit size and offset is calculated when the
   * measurement is initialized.
   * @param bits Offset to data.
   */
  virtual void BitOffset(uint16_t bits) = 0;

  /** \brief Returns offset to data (0..7). */
  [[nodiscard]] virtual uint16_t BitOffset() const = 0;

  /** \brief Sets the byte offset in record to to data.
   *
   * This function shall only be used for MDF4 files and for composition
   * channels. Composition channels reference their parent byte array channel.
   * For ordinary channels, the bit size and offset is calculated when the
   * measurement is initialized.
   * @param bytes Offset to data.
   */
  virtual void ByteOffset(uint32_t bytes) = 0;

  /** \brief Returns the byte offset to data in the record. */
  [[nodiscard]] virtual uint32_t ByteOffset() const = 0;
 protected:

  /** \brief Support function that copies a record to a data block. */
  virtual void CopyToDataBuffer(const std::vector<uint8_t> &record_buffer,
                                std::vector<uint8_t> &data_buffer) const;

  /** \brief Support function that get signed integer from a record. */
  virtual bool GetSignedValue(const std::vector<uint8_t> &record_buffer,
                              int64_t &dest) const;
  /** \brief Support function that get float values from a record. */
  virtual bool GetFloatValue(const std::vector<uint8_t> &record_buffer,
                             double &dest) const;

  /** \brief Support function that get array values from a record. */
  virtual bool GetByteArrayValue(const std::vector<uint8_t> &record_buffer,
                                 std::vector<uint8_t> &dest) const;

  /** \brief Support function that get CANOpen date values from a record. */
  virtual bool GetCanOpenDate(const std::vector<uint8_t> &record_buffer,
                              uint64_t &dest) const;

  /** \brief Support function that get CANOpen time values from a record. */
  virtual bool GetCanOpenTime(const std::vector<uint8_t> &record_buffer,
                              uint64_t &dest) const;

  /** \brief Support function that gets the channel group sample buffer */
  [[nodiscard]] virtual std::vector<uint8_t> &SampleBuffer() const = 0;

  /** \brief Support function that sets the valid flag. */
  virtual void SetValid(bool valid);

  /** \brief Support function that return true if the valid bit is set.*/
  virtual bool GetValid(const std::vector<uint8_t> &record_buffer) const;

  /** \brief Support function that sets unsigned little endian values */
  void SetUnsignedValueLe(uint64_t value, bool valid);
  /** \brief Support function that sets unsigned big endian values */
  void SetUnsignedValueBe(uint64_t value, bool valid);
  /** \brief Support function that sets signed little endian values */
  void SetSignedValueLe(int64_t value, bool valid);
  /** \brief Support function that sets signed big endian values */
  void SetSignedValueBe(int64_t value, bool valid);
  /** \brief Support function that sets float little endian values */
  void SetFloatValueLe(double value, bool valid);
  /** \brief Support function that sets float big endian values */
  void SetFloatValueBe(double value, bool valid);
  /** \brief Support function that sets text values */
  virtual void SetTextValue(const std::string &value, bool valid);
  /** \brief Support function that sets array values */
  virtual void SetByteArray(const std::vector<uint8_t> &value, bool valid);

 private:

  mutable uint64_t vlsd_record_id_ = 0; ///< Used to fix the VLSD CG block.
};

template <typename T>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               T &dest) const {
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
    default:
      break;
  }
  if (valid) {
     valid = GetValid(record_buffer);
  }
  return valid;
}

/** \brief Support function that gets an array value from a record buffer. */
template <>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               std::vector<uint8_t> &dest) const;

/** \brief Support function that gets a string value from a record buffer. */
template <>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               std::string &dest) const;

template <typename T>
void IChannel::SetChannelValue(const T &value, bool valid) {
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
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF16Le:
      SetTextValue(std::to_string(value), valid);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      // SetByteArray(value, valid);
      break;

    case ChannelDataType::CanOpenDate:
      if (typeid(T) == typeid(uint64_t) && DataBytes() == 7) {
        const auto date_array =
            MdfHelper::NsToCanOpenDateArray(static_cast<uint64_t>(value));
        SetByteArray(date_array, valid);
      } else {
        SetValid(false);
      }
      break;

    case ChannelDataType::CanOpenTime:
      if (typeid(T) == typeid(uint64_t) && DataBytes() == 6) {
        const auto time_array =
            MdfHelper::NsToCanOpenTimeArray(static_cast<uint64_t>(value));
        SetByteArray(time_array, valid);
      } else {
        SetValid(false);
      }
      break;


    default:
      SetValid(false);
      break;
  }
};

/** \brief Support function that sets a string value to a record buffer. */
template <>
void IChannel::SetChannelValue(const std::string &value, bool valid);

/** \brief Support function that sets a string value to a record buffer. */
template <>
void IChannel::SetChannelValue(const std::vector<uint8_t> &value, bool valid);
}  // namespace mdf
