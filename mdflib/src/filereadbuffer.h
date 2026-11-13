/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <streambuf>

namespace mdf::detail {

/** \brief Read-only stream buffer that keeps the last pages of the file.
 *
 * A std::filebuf drops its buffer on every seek. Reading the block headers of
 * an MF4 file jumps back and forth between a few places, so most of these
 * seeks land in a page that was read a moment ago. This buffer keeps the last
 * pages and answers such a seek from memory. The file itself belongs to a
 * std::filebuf below, so opening and closing stay unchanged.
 */
class FileReadBuffer : public std::streambuf {
 public:
  FileReadBuffer();

  /** \brief Returns the buffer that owns the file. Use it to open and close. */
  std::filebuf& File() { return file_; }

  /** \brief Drops the pages. Needed when the file is opened again. */
  void Reset();

 protected:
  int_type underflow() override;
  std::streamsize xsgetn(char* data, std::streamsize count) override;
  pos_type seekoff(off_type offset, std::ios_base::seekdir dir,
                   std::ios_base::openmode which) override;
  pos_type seekpos(pos_type pos, std::ios_base::openmode which) override;

 private:
  static constexpr std::streamsize kPageSize = 4096;
  static constexpr size_t kPages = 8;

  struct Page {
    std::array<char, static_cast<size_t>(kPageSize)> data;
    int64_t pos = -1;            ///< Position in the file, -1 while unused
    std::streamsize filled = 0;  ///< Number of valid bytes in the page
  };

  std::filebuf file_;
  std::array<Page, kPages> page_;
  size_t next_ = 0;       ///< Page that is filled next
  int64_t page_pos_ = 0;  ///< File position of the first byte of the get area
  int64_t file_pos_ = 0;  ///< Read position of the file buffer below
  char empty_ = 0;        ///< Stands in for the get area while no page is used

  int64_t Position() const { return page_pos_ + (gptr() - eback()); }
  bool Reach(int64_t position);
  pos_type Move(int64_t position);
};

}  // namespace mdf::detail
