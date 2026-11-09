/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <string>
#include <atomic>
#include <cstdint>

#include "mdf/ichannelobserver.h"

namespace mdf {

class MdfReader;
class ISignal {
public:
  ISignal() = default;
  virtual ~ISignal() = default;
  void SetSelected(bool selected) { selected_ = selected; }
  [[nodiscard]] bool IsSelected() const { return selected_; }
  [[nodiscard]] bool HaveData() const { return have_data_; }

  [[nodiscard]] const std::string& Name() const { return name_; }

  [[nodiscard]] virtual std::string Description() const = 0;
  [[nodiscard]] virtual std::string Unit() const = 0;
  [[nodiscard]] virtual size_t NofSamples() const = 0;
  [[nodiscard]] virtual std::string Source() const = 0;

  [[nodiscard]] virtual MdfReader* Reader() const = 0;
  [[nodiscard]] virtual int64_t BlockIndex() const = 0;

  [[nodiscard]] virtual const IChannelObserver* SignalData() const = 0;
  [[nodiscard]] virtual const IChannelObserver* MasterData() const = 0;


protected:
  std::atomic<bool> selected_ = false;
  std::atomic<bool> have_data_ = false;
  std::string name_;

};

}  // namespace mdf

