/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include "ichannel.h"
#include "samplerecord.h"
namespace mdf {

class IChannelGroup {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  [[nodiscard]] virtual uint64_t RecordId() const = 0;

  virtual void Name(const std::string& name) = 0;
  [[nodiscard]] virtual std::string Name() const = 0;

  virtual void Description(const std::string& description) = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  [[nodiscard]] virtual uint64_t NofSamples() const = 0;
  virtual void NofSamples(uint64_t nof_samples) = 0;

  [[nodiscard]] virtual std::vector<IChannel *> Channels() const = 0;
  [[nodiscard]] virtual const IChannel* GetXChannel(const IChannel& reference) const = 0;

  [[nodiscard]] SampleRecord GetSampleRecord() const;

  void ResetSample() const;
  void IncrementSample() const;
  [[nodiscard]] size_t Sample() const;
 protected:
  mutable std::vector<uint8_t> sample_buffer_; ///< Temporary record when saving samples.
 private:
  mutable size_t sample_ = 0; ///< Support for the OnSample observers

};


}
