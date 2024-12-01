/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf4writer.h"
#include <fstream>
namespace mdf::detail {

class MdfConverter : public Mdf4Writer {
 public:
  MdfConverter() = default;
  ~MdfConverter() override;

  bool InitMeasurement() override;
  void SaveSample(const IChannelGroup& group, uint64_t time) override;
  bool FinalizeMeasurement() override;
 protected:
  void TrimQueue() override; ///< Trims the sample queue.
  void SaveQueue(std::unique_lock<std::mutex>& lock) override;
  void CleanQueue(std::unique_lock<std::mutex>& lock) override;

  void SaveQueueCompressed(std::unique_lock<std::mutex>& lock) override;

  /** \brief Save one DZ block from the sample queue. */
  void CleanQueueCompressed(std::unique_lock<std::mutex>& lock,
                            bool finalize) override;
 private:

  void ConverterThread();
};

}  // namespace mdf::detail

