/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstdint>
#include <fstream>
#include <ios>
#include <memory>
#include <streambuf>
#include <vector>

namespace mdf {

/** \brief Read-only stream buffer that keeps recently read pages of the file.
 *
 * A std::filebuf drops its buffer on every seek. Reading the block headers of
 * an MF4 file jumps back and forth between a few places, so most of these
 * seeks land in a page that was read a moment ago. This buffer keeps up to
 * 1024 recently read 4 kB pages (4 MiB per buffer, allocated on the first
 * buffered read) and answers such a seek from memory, which makes
 * ReadEverythingButData() faster on files with many channels. MdfReader uses a
 * std::filebuf by default.
 *
 * The file itself belongs to a std::filebuf below. Open it there and pass the
 * buffer to MdfReader, which does not open or close it.
 * \code
 * auto buffer = std::make_shared<mdf::FileReadBuffer>();
 * buffer->File().open(filename, std::ios_base::in | std::ios_base::binary);
 * mdf::MdfReader reader(buffer);
 * \endcode
 */
class FileReadBuffer : public std::streambuf {
 public:
  FileReadBuffer() = default;
  FileReadBuffer(const FileReadBuffer&) = delete;
  FileReadBuffer& operator=(const FileReadBuffer&) = delete;

  /** \brief Returns the buffer that owns the file.
   *
   * Only open and close the file through it, in binary mode, read only while
   * it is open, and call Reset() after opening it again. Reading starts at 0,
   * also with ate.
   */
  std::filebuf& File() { return file_; }

  /** \brief Drops the pages and goes back to the start of the file. */
  void Reset();

 protected:
  int_type underflow() override;
  std::streamsize xsgetn(char* data, std::streamsize count) override;
  pos_type seekoff(off_type offset, std::ios_base::seekdir dir,
                   std::ios_base::openmode which) override;
  pos_type seekpos(pos_type pos, std::ios_base::openmode which) override;

 private:
  static constexpr std::streamsize kPageSize = 4096;
  static constexpr size_t kPages = 1024;

  struct Page {
    int64_t pos = -1;            ///< Position in the file, -1 while unused
    std::streamsize filled = 0;  ///< Number of valid bytes in the page
  };

  std::filebuf file_;
  /// A file page n is kept in page_[n % kPages], so finding it needs no search.
  /// Both are allocated on the first buffered read, empty until then.
  std::vector<Page> page_;
  std::unique_ptr<char[]> data_;  ///< Page bytes
  int64_t page_pos_ = 0;   ///< File position of the first byte of the get area
  int64_t file_pos_ = -1;  ///< Position of the file buffer below, -1 if unknown

  int64_t Position() const { return page_pos_ + (gptr() - eback()); }
  pos_type Move(int64_t position);
  bool Reach(int64_t position);
  char* Data(const Page& page) {
    return data_.get() + (&page - page_.data()) * kPageSize;
  }
};

}  // namespace mdf
