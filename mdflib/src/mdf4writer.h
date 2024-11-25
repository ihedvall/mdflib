/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdfwriter.h"

namespace mdf::detail {
class Dg4Block;

class Mdf4Writer : public MdfWriter {
 public:
  Mdf4Writer() = default;
  ~Mdf4Writer() override;

  bool InitMeasurement() override;

  IChannelConversion* CreateChannelConversion(IChannel* parent) override;

 protected:
  uint64_t offset_ = 0;
  void CreateMdfFile() override;
  void SetLastPosition(std::streambuf& buffer) override;
  bool PrepareForWriting() override;
  void SaveQueue(std::unique_lock<std::mutex>& lock) override;
  void CleanQueue(std::unique_lock<std::mutex>& lock) override;

  /** \brief Calculates number of DZ blocks in the sample queue */
  [[nodiscard]] size_t CalculateNofDzBlocks();
  virtual void SaveQueueCompressed(std::unique_lock<std::mutex>& lock);

  /** \brief Save one DZ block from the sample queue. */
  virtual void CleanQueueCompressed(std::unique_lock<std::mutex>& lock, bool finalize);

  void SetDataPosition(std::streambuf& buffer) override;
  bool WriteSignalData(std::streambuf& buffer) override;

  Dg4Block* GetLastDg4();
 private:


};

}  // namespace mdf::detail
