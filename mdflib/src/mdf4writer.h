/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdfwriter.h"

namespace mdf::detail {

class Mdf4Writer : public MdfWriter {
 public:
  Mdf4Writer() = default;
  ~Mdf4Writer() override;

  bool InitMeasurement() override;

  IChannelConversion* CreateChannelConversion(IChannel* parent) override;

 protected:
  void CreateMdfFile() override;
  void SetLastPosition(std::FILE* file) override;
  bool PrepareForWriting() override;
  void SaveQueue(std::unique_lock<std::mutex>& lock) override;
  void CleanQueue(std::unique_lock<std::mutex>& lock) override;
  void SetDataPosition(std::FILE* file) override;
  bool WriteSignalData(std::FILE* file) override;
 private:
  time_t save_timer_ = 0;
  uint64_t offset_ = 0;
  [[nodiscard]] size_t CalculateNofDzBlocks();
  void SaveQueueCompressed(std::unique_lock<std::mutex>& lock);
  void CleanQueueCompressed(std::unique_lock<std::mutex>& lock);
};

}  // namespace mdf::detail
