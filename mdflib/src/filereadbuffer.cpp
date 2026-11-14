/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "filereadbuffer.h"

#include <algorithm>
#include <cstring>

namespace mdf::detail {

FileReadBuffer::FileReadBuffer() { Reset(); }

void FileReadBuffer::Reset() {
  for (Page& page : page_) {
    page.pos = -1;
    page.filled = 0;
  }
  next_ = 0;
  page_pos_ = 0;
  file_pos_ = 0;
  setg(&empty_, &empty_, &empty_);
}

/* The file buffer below is only moved through this function, so file_pos_
 * always holds its read position. A value that does not match it would read at
 * the wrong place. */
bool FileReadBuffer::Reach(int64_t position) {
  if (file_pos_ == position) {
    return true;
  }
  if (file_.pubseekpos(static_cast<pos_type>(position), std::ios_base::in) <
      pos_type(0)) {
    return false;
  }
  file_pos_ = position;
  return true;
}

std::streambuf::int_type FileReadBuffer::underflow() {
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }
  const int64_t wanted = Position();
  // A page starts at a multiple of its size. Pages therefore never overlap,
  // and a place that is read again is covered by the same page.
  const int64_t start = wanted - wanted % kPageSize;
  Page& page = page_[next_];
  next_ = (next_ + 1) % kPages;
  page.pos = -1;
  if (!Reach(start)) {
    return traits_type::eof();
  }
  page.filled = file_.sgetn(page.data.data(), kPageSize);
  if (page.filled > 0) {
    file_pos_ += page.filled;
  }
  if (page.filled <= wanted - start) {
    return traits_type::eof();
  }
  page.pos = start;
  page_pos_ = start;
  setg(page.data.data(), page.data.data() + (wanted - start),
       page.data.data() + page.filled);
  return traits_type::to_int_type(*gptr());
}

std::streamsize FileReadBuffer::xsgetn(char* data, std::streamsize count) {
  // The usual case while the block headers are read: the request is already
  // in the page, so it is served without a call into the base class.
  if (count <= egptr() - gptr()) {
    std::memcpy(data, gptr(), static_cast<size_t>(count));
    gbump(static_cast<int>(count));
    return count;
  }
  if (count < kPageSize) {
    return std::streambuf::xsgetn(data, count);
  }
  // A block of a page or more goes straight into the buffer of the caller.
  // The bytes that are still in the page are taken first, because the file
  // buffer below stands behind them.
  std::streamsize done = std::min<std::streamsize>(egptr() - gptr(), count);
  if (done > 0) {
    std::memcpy(data, gptr(), static_cast<size_t>(done));
    gbump(static_cast<int>(done));
  }
  const int64_t position = Position();
  if (done >= count || !Reach(position)) {
    return done;
  }
  const std::streamsize read = file_.sgetn(data + done, count - done);
  if (read > 0) {
    file_pos_ += read;
    page_pos_ = position + read;
    setg(&empty_, &empty_, &empty_);
    done += read;
  }
  return done;
}

std::streambuf::pos_type FileReadBuffer::Move(int64_t position) {
  if (position < 0) {
    return pos_type(off_type(-1));
  }
  for (Page& page : page_) {
    if (page.pos >= 0 && position >= page.pos &&
        position < page.pos + page.filled) {
      page_pos_ = page.pos;
      setg(page.data.data(), page.data.data() + (position - page.pos),
           page.data.data() + page.filled);
      return pos_type(position);
    }
  }
  page_pos_ = position;
  setg(&empty_, &empty_, &empty_);
  return pos_type(position);
}

std::streambuf::pos_type FileReadBuffer::seekoff(
    off_type offset, std::ios_base::seekdir dir,
    std::ios_base::openmode which) {
  if ((which & std::ios_base::in) == 0) {
    return pos_type(off_type(-1));
  }
  if (dir == std::ios_base::cur) {
    return Move(Position() + offset);
  }
  if (dir == std::ios_base::beg) {
    return Move(offset);
  }
  const pos_type end = file_.pubseekoff(offset, std::ios_base::end, which);
  if (end < pos_type(0)) {
    return end;
  }
  file_pos_ = static_cast<int64_t>(off_type(end));
  return Move(file_pos_);
}

std::streambuf::pos_type FileReadBuffer::seekpos(
    pos_type pos, std::ios_base::openmode which) {
  if ((which & std::ios_base::in) == 0) {
    return pos_type(off_type(-1));
  }
  return Move(static_cast<int64_t>(off_type(pos)));
}

}  // namespace mdf::detail
