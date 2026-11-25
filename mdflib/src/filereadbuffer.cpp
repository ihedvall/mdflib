/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/filereadbuffer.h"

namespace mdf {

void FileReadBuffer::Reset() {
  for (Page& page : page_) {
    page.pos = -1;
  }
  page_pos_ = 0;
  file_pos_ = -1;
  setg(nullptr, nullptr, nullptr);
}

/* A seek drops the read area of the file buffer below. With libstdc++ on
 * Linux that area is 8 kB, so reading pages one after another would read
 * twice as much. The seek is left out when the file buffer already stands at
 * the position. */
bool FileReadBuffer::Reach(int64_t position) {
  if (file_pos_ == position) {
    return true;
  }
  if (file_.pubseekpos(position, std::ios_base::in) < pos_type(0)) {
    return false;
  }
  file_pos_ = position;
  return true;
}

std::streambuf::int_type FileReadBuffer::underflow() {
  const int64_t wanted = Position();
  Move(wanted);  // The next page may still be kept
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }
  // A page starts at a multiple of its size. Pages therefore never overlap,
  // and a place that is read again is covered by the same page.
  const int64_t start = wanted - wanted % kPageSize;
  if (page_.empty()) {
    data_.reset(new char[kPages * kPageSize]);
    page_.resize(kPages);
  }
  Page& page = page_[static_cast<size_t>(start / kPageSize) % kPages];
  page.pos = -1;
  if (!Reach(start)) {
    return traits_type::eof();
  }
  page.filled = file_.sgetn(Data(page), kPageSize);
  file_pos_ += page.filled;
  if (page.filled <= wanted - start) {
    return traits_type::eof();
  }
  page.pos = start;
  Move(wanted);
  return traits_type::to_int_type(*gptr());
}

std::streamsize FileReadBuffer::xsgetn(char* data, std::streamsize count) {
  if (count < kPageSize) {
    return std::streambuf::xsgetn(data, count);
  }
  // A block of a page or more goes straight into the buffer of the caller
  const int64_t position = Position();
  if (!Reach(position)) {
    return 0;
  }
  const std::streamsize read = file_.sgetn(data, count);
  file_pos_ += read;
  Move(position + read);
  return read;
}

std::streambuf::pos_type FileReadBuffer::Move(int64_t position) {
  if (position < 0) {
    return pos_type(off_type(-1));
  }
  page_pos_ = position;
  setg(nullptr, nullptr, nullptr);
  if (page_.empty()) {
    return pos_type(position);  // No page read yet
  }
  Page& page = page_[static_cast<size_t>(position / kPageSize) % kPages];
  if (page.pos >= 0 && position >= page.pos &&
      position < page.pos + page.filled) {
    page_pos_ = page.pos;
    setg(Data(page), Data(page) + (position - page.pos),
         Data(page) + page.filled);
  }
  return pos_type(position);
}

std::streambuf::pos_type FileReadBuffer::seekoff(off_type offset,
                                                 std::ios_base::seekdir dir,
                                                 std::ios_base::openmode) {
  if (dir == std::ios_base::cur) {
    return Move(Position() + offset);
  }
  if (dir == std::ios_base::beg) {
    return Move(offset);
  }
  const pos_type end =
      file_.pubseekoff(offset, std::ios_base::end, std::ios_base::in);
  file_pos_ = off_type(end);  // -1 if the seek failed
  return end < pos_type(0) ? end : Move(file_pos_);
}

std::streambuf::pos_type FileReadBuffer::seekpos(
    pos_type pos, std::ios_base::openmode which) {
  return seekoff(off_type(pos), std::ios_base::beg, which);
}

}  // namespace mdf
