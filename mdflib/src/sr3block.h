/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>

#include "datalistblock.h"
#include "mdf/isamplereduction.h"

namespace mdf::detail {
class Sr3Block : public DataListBlock , public ISampleReduction {
 public:
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override;

  void NofSamples(uint64_t nof_samples) override;
  [[nodiscard]] uint64_t NofSamples() const override;

  void Interval(double interval) override;
  [[nodiscard]] double Interval() const override;

  [[nodiscard]] const IChannelGroup* ChannelGroup() const override;

  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  void ReadData(std::FILE *file) const;
  void ClearData() override;

  void GetChannelValueUint( const IChannel& channel, uint64_t sample,
                            uint64_t array_index, SrValue<uint64_t>& value ) const override;
  void GetChannelValueInt( const IChannel& channel, uint64_t sample,
                           uint64_t array_index, SrValue<int64_t>& value ) const override;
  void GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                              uint64_t array_index, SrValue<double>& value ) const override;
 private:
  uint32_t nof_reduced_samples_ = 0;
  double time_interval_ = 0;
  mutable std::vector<uint8_t> data_list_;
};

}  // namespace mdf::detail
