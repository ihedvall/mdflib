/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file idatawriter.h
 * \brief Interface for channel-group record buffer writer.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "mdf/ichannel.h"
#include "mdf/samplerecord.h"

namespace mdf {

/** \brief Immutable layout metadata for one channel in a record buffer. */
struct DataWriterChannelLayout {
  size_t channel_index = 0;
  std::string name;
  uint32_t byte_offset = 0;
  uint64_t data_bytes = 0;
  ChannelDataType data_type = ChannelDataType::UnsignedIntegerLe;
  void* user_object = nullptr;
};

/** \brief Interface used to build one serialized sample buffer.
 *
 * The writer only builds the raw record buffer. It does not save samples or
 * write data to file. Persisting a sample is still done by calling
 * MdfWriter::SaveSample() on the owning channel group.
 */
class IDataWriter {
 public:
  virtual ~IDataWriter() = default;

  /** \brief Clears the current working buffer. */
  virtual void Reset() = 0;

  /** \brief Returns writable record buffer used for direct serialization. */
  [[nodiscard]] virtual std::vector<uint8_t>& Buffer() = 0;

  /** \brief Returns read-only record buffer used for direct serialization. */
  [[nodiscard]] virtual const std::vector<uint8_t>& Buffer() const = 0;

  /** \brief Returns expected fixed record size in bytes. */
  [[nodiscard]] virtual size_t RecordSize() const = 0;

  /** \brief Copies an externally serialized record into the working buffer.
   *
   * @return True on success.
   */
  [[nodiscard]] virtual bool WriteRawRecord(const void* buffer,
                                            size_t length) = 0;

  /** \brief Copies the working buffer into the channel group sample buffer.
   *
   * This method does not enqueue or save a sample.
   * @return True on success.
   */
  [[nodiscard]] virtual SampleRecord Commit() = 0;

  /** \brief Returns channel layout metadata in channel index order. */
  [[nodiscard]] virtual const std::vector<DataWriterChannelLayout>& ChannelLayouts()
      const = 0;

  template <typename T>
  void SetValue(const DataWriterChannelLayout& layout, T value) {
    std::vector<uint8_t>& buffer = Buffer();

    if (layout.data_bytes != sizeof(T)) {
      return;
    }
    if (layout.byte_offset + layout.data_bytes > buffer.size()) {
      return;
    }
    switch (layout.data_type) {
      case ChannelDataType::UnsignedIntegerLe:
      case ChannelDataType::SignedIntegerLe:
      case ChannelDataType::FloatLe:
        memcpy(buffer.data() + layout.byte_offset, &value, layout.data_bytes);
        if (!IsLittleEndian()) {
          std::reverse(buffer.begin() + layout.byte_offset,
            buffer.begin() + layout.byte_offset + layout.data_bytes);
        }
        break;

      case ChannelDataType::UnsignedIntegerBe:
      case ChannelDataType::SignedIntegerBe:
      case ChannelDataType::FloatBe:
        memcpy(buffer.data() + layout.byte_offset, &value, layout.data_bytes);
        if (IsLittleEndian()) {
          std::reverse(buffer.begin() + layout.byte_offset,
            buffer.begin() + layout.byte_offset + layout.data_bytes);
        }
        break;

      default:
        break;;
    }
  }

private:
  [[nodiscard]] static constexpr bool IsLittleEndian() {
    constexpr int num = 1;
    return *(char*) &num == 1;
  }
};


}  // namespace mdf
