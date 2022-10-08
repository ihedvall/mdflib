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
namespace CgFlag {
constexpr uint16_t VlsdChannel = 0x0001;
constexpr uint16_t BusEvent = 0x0002;
constexpr uint16_t PlainBusEvent = 0x0004;
constexpr uint16_t RemoteMaster = 0x0008;
constexpr uint16_t EventSignal = 0x00010;
}  // namespace CgFlag

class ISourceInformation;

class IChannelGroup {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void RecordId(uint64_t record_id) = 0;
  [[nodiscard]] virtual uint64_t RecordId() const = 0;

  virtual void Name(const std::string& name) = 0;
  [[nodiscard]] virtual std::string Name() const = 0;

  virtual void Description(const std::string& description) = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  [[nodiscard]] virtual uint64_t NofSamples() const = 0;
  virtual void NofSamples(uint64_t nof_samples) = 0;

  [[nodiscard]] virtual uint16_t Flags();
  virtual void Flags(uint16_t flags);

  [[nodiscard]] virtual char16_t PathSeparator();
  virtual void PathSeparator(char16_t path_separator);

  [[nodiscard]] virtual std::vector<IChannel*> Channels() const = 0;
  [[nodiscard]] virtual const IChannel* GetXChannel(
      const IChannel& reference) const = 0;

  [[nodiscard]] virtual ISourceInformation* CreateSourceInformation();
  [[nodiscard]] virtual const ISourceInformation* SourceInformation() const;

  [[nodiscard]] SampleRecord GetSampleRecord() const;

  void ResetSample() const;
  void IncrementSample() const;
  [[nodiscard]] size_t Sample() const;

 protected:
  mutable std::vector<uint8_t>
      sample_buffer_;  ///< Temporary record when saving samples.
 private:
  mutable size_t sample_ = 0;  ///< Support for the OnSample observers
};

}  // namespace mdf
