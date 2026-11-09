/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <string>
#include <memory>
#include <deque>

#include "mdf/mdfreader.h"
#include "mdf/isignal.h"

namespace mdf {

using SignalList = std::deque<std::unique_ptr<ISignal>>;
using ReaderList = std::deque<std::unique_ptr<MdfReader>>;

class ReaderThread {
public:
  ReaderThread() = default;
  virtual ~ReaderThread() = default;

  bool AddMdfFile(const std::wstring& filename);
  [[nodiscard]] SignalList& GetSignalList() { return signal_list_; }
  [[nodiscard]] const SignalList& GetSignalList() const { return signal_list_; }

  [[nodiscard]] ReaderList& GetReaderList() { return reader_list_; }
  [[nodiscard]] const ReaderList& GetReaderList() const { return reader_list_; }
protected:
  std::deque<std::unique_ptr<MdfReader>> reader_list_;
  std::deque<std::unique_ptr<ISignal>> signal_list_;
private:

  void CreateUniqueName(MdfReader& reader) const;
  bool CreateSignals(MdfReader& reader);
};

}  // namespace mdf


