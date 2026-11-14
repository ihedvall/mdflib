/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "filereadbuffer.h"

using mdf::detail::FileReadBuffer;

namespace {

constexpr std::ios_base::openmode kRead =
    std::ios_base::in | std::ios_base::binary;

constexpr int64_t kPage = 4'096;

// The buffer keeps eight pages of 4 kB. Every test uses its own file name so
// that the tests can run in parallel.
std::string MakeTestFile(const std::string_view& name, size_t size,
                         size_t shift = 0) {
  std::filesystem::path temp = std::filesystem::temp_directory_path();
  temp.append("test");
  std::filesystem::create_directories(temp);
  temp.append(name);

  std::ofstream file(
      temp, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  for (size_t index = 0; index < size; ++index) {
    file.put(static_cast<char>((index + shift) % 251));
  }
  file.close();
  return temp.string();
}

char Expected(size_t position, size_t shift = 0) {
  return static_cast<char>((position + shift) % 251);
}

// The same steps on any stream buffer. The text is compared against the text of
// a std::filebuf, so the new buffer has to behave the same way.
std::string Walk(std::streambuf& buffer, int64_t size) {
  std::ostringstream trace;
  // One byte at the start fills the first page, so the file below stands at
  // the end of it. A block of more than a page from an unaligned place behind
  // it has to seek. A buffer that holds that position one byte wrong reads
  // bytes that still look plausible.
  std::vector<char> ahead(9'000);
  trace << "first=" << buffer.sbumpc()
        << " at=" << buffer.pubseekpos(kPage + 1, std::ios_base::in);
  const auto unaligned =
      buffer.sgetn(ahead.data(), static_cast<std::streamsize>(ahead.size()));
  trace << " unaligned=" << unaligned;
  for (std::streamsize index = 0; index < unaligned; ++index) {
    trace << "," << static_cast<int>(ahead[static_cast<size_t>(index)]);
  }
  trace << " end="
        << buffer.pubseekoff(0, std::ios_base::end, std::ios_base::in)
        << " beg=" << buffer.pubseekpos(0, std::ios_base::in);
  int64_t sum = 0;
  int64_t count = 0;
  for (auto value = buffer.sbumpc();
       value != std::streambuf::traits_type::eof(); value = buffer.sbumpc()) {
    sum += static_cast<unsigned char>(value);
    ++count;
  }
  trace << " read=" << count << " sum=" << sum << " tell="
        << buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in);
  // The last byte of a page, the first behind it and the one after that,
  // and twice a return to the start of an earlier fill. A file buffer below
  // that is not moved along would read at the wrong place.
  for (const int64_t position :
       {int64_t{0}, size / 2, size - 1, size, size + 10, int64_t{4'095},
        int64_t{4'096}, int64_t{4'097}, int64_t{0}, int64_t{4'096},
        int64_t{0}}) {
    trace << " [" << position << "->"
          << buffer.pubseekpos(position, std::ios_base::in) << ":"
          << buffer.sbumpc() << "]";
  }
  // Leaving a page without reading anything, then back to the start of the
  // last fill, and once with a read in between
  for (const bool read_away : {false, true}) {
    buffer.pubseekpos(0, std::ios_base::in);
    trace << " fill=" << buffer.sbumpc();
    buffer.pubseekpos(size > 9'000 ? 9'000 : size, std::ios_base::in);
    if (read_away) {
      trace << "," << buffer.sbumpc();
    }
    buffer.pubseekpos(0, std::ios_base::in);
    trace << "," << buffer.sbumpc();
    buffer.pubseekpos(4'096, std::ios_base::in);
    trace << "," << buffer.sbumpc();
  }
  // More pages than the buffer holds, then the same pages shifted by one
  // byte. A page that was dropped in between has to be read again, and one
  // that is still there has to answer at the right place.
  for (const int64_t shift : {int64_t{0}, int64_t{1}}) {
    for (int64_t page = 0; page < 11; ++page) {
      const int64_t at = page * kPage + shift;
      trace << " p" << at << "=" << buffer.pubseekpos(at, std::ios_base::in)
            << ":" << buffer.sbumpc();
    }
  }
  // Blocks smaller than a page, exactly a page and larger than it.
  // Each one starts at the begin of the file, in the middle and near the end,
  // and once after a single byte was taken out of the page.
  trace << " back=" << buffer.pubseekpos(-1, std::ios_base::in);
  for (const size_t block_size : {size_t{50}, size_t{4096}, size_t{9000}}) {
    for (const int64_t start :
         {int64_t{0}, size / 2, size > 20 ? size - 20 : 0}) {
      for (const bool peek : {false, true}) {
        std::vector<char> block(block_size);
        buffer.pubseekpos(start, std::ios_base::in);
        if (peek) {
          // Leaves the page filled and partly read
          buffer.sbumpc();
        }
        const auto got = buffer.sgetn(
            block.data(), static_cast<std::streamsize>(block.size()));
        trace << " get" << block_size << "@" << start << (peek ? "p" : "")
              << "=" << got;
        for (std::streamsize index = 0; index < got; ++index) {
          trace << "," << static_cast<int>(block[static_cast<size_t>(index)]);
        }
        // The position behind the block, then the last byte of the block once
        // more, then the byte behind it. A file buffer below that is not moved
        // along would read at the wrong place.
        trace << "|"
              << buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in);
        if (got > 0) {
          buffer.pubseekpos(start + (peek ? 1 : 0) + got - 1,
                            std::ios_base::in);
          trace << "|" << buffer.sbumpc();
        }
        trace << "|" << buffer.sbumpc();
      }
    }
  }
  return trace.str();
}

}  // namespace

namespace mdf::test {

TEST(FileReadBuffer, SameAsFileBuffer) {
  // Smaller than a page, exactly a page, larger than a page, and larger than
  // all pages together
  for (const size_t size :
       {size_t{0}, size_t{1}, size_t{100}, size_t{4095}, size_t{4096},
        size_t{4097}, size_t{9000}, size_t{40'000}}) {
    const std::string filename = MakeTestFile("filereadbuffer_same.bin", size);
    std::string plain_trace;
    std::string buffer_trace;
    {
      std::filebuf plain;
      ASSERT_NE(plain.open(filename, kRead), nullptr);
      plain_trace = Walk(plain, static_cast<int64_t>(size));
      plain.close();
    }
    {
      FileReadBuffer buffer;
      ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
      buffer_trace = Walk(buffer, static_cast<int64_t>(size));
      buffer.File().close();
    }
    EXPECT_EQ(buffer_trace, plain_trace) << "file size " << size;
    std::error_code error;
    std::filesystem::remove(filename, error);
  }
}

TEST(FileReadBuffer, PageOutOfOrder) {
  // Three pages in an order where the number of bytes read so far is the same
  // as the start of the next page. A buffer that does not keep track of where
  // the file below stands would leave out the seek and read at the wrong
  // place, and the bytes would still look plausible.
  const std::string filename = MakeTestFile("filereadbuffer_order.bin", 40'000);
  FileReadBuffer buffer;
  ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
  for (const int64_t at : {int64_t{0}, 4 * kPage, 2 * kPage}) {
    ASSERT_EQ(buffer.pubseekpos(at, std::ios_base::in), at);
    EXPECT_EQ(static_cast<char>(buffer.sbumpc()),
              Expected(static_cast<size_t>(at)))
        << "page at " << at;
  }

  buffer.File().close();
  std::error_code error;
  std::filesystem::remove(filename, error);
}

TEST(FileReadBuffer, ResetAfterReopen) {
  const std::string filename =
      MakeTestFile("filereadbuffer_reopen.bin", 10'000);
  FileReadBuffer buffer;
  ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
  EXPECT_EQ(static_cast<char>(buffer.sbumpc()), Expected(0));
  buffer.File().close();

  // The same name with other bytes. Reading the place that was read above
  // again shows whether the pages are gone, because a page that stayed would
  // answer with the bytes of the first file.
  MakeTestFile("filereadbuffer_reopen.bin", 10'000, 7);
  buffer.Reset();
  ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
  EXPECT_EQ(buffer.pubseekoff(0, std::ios_base::cur, std::ios_base::in), 0);
  EXPECT_EQ(static_cast<char>(buffer.sbumpc()), Expected(0, 7));
  buffer.File().close();

  // Reading behind the first page right away, and that page ends where the
  // file buffer below stood before the close. A reset that forgets its
  // position would leave out the seek and read at the start of the file.
  buffer.Reset();
  ASSERT_NE(buffer.File().open(filename, kRead), nullptr);
  EXPECT_EQ(buffer.pubseekpos(kPage, std::ios_base::in), kPage);
  EXPECT_EQ(static_cast<char>(buffer.sbumpc()),
            Expected(static_cast<size_t>(kPage), 7));

  buffer.File().close();
  std::error_code error;
  std::filesystem::remove(filename, error);
}

}  // end namespace mdf::test
