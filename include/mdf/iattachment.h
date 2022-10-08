/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <optional>
#include <string>
#include <vector>

namespace mdf {

class IAttachment {
 public:
  /** \brief Returns the file position.
   *
   * Returns the file position of the block. This index is updated when
   * the block is saved onto the file. The index is also used to indicate
   * if the block has been written to the file. Note that attachment blocks
   * are not updated once written to the file.
   * @return File position
   */
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void CreatorIndex(uint16_t creator) = 0;
  [[nodiscard]] virtual uint16_t CreatorIndex() const = 0;

  virtual void IsEmbedded(bool embed) = 0;
  [[nodiscard]] virtual bool IsEmbedded() const = 0;

  virtual void IsCompressed(bool compress) = 0;
  [[nodiscard]] virtual bool IsCompressed() const = 0;

  [[nodiscard]] virtual std::optional<std::string> Md5() const = 0;

  virtual void FileName(const std::string& filename) = 0;
  [[nodiscard]] virtual const std::string& FileName() const = 0;

  virtual void FileType(const std::string& file_type) = 0;
  [[nodiscard]] virtual const std::string& FileType() const = 0;
};

}  // namespace mdf
