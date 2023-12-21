/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "datalistblock.h"
#include "mdf/isamplereduction.h"
#include "mdf/ichannelgroup.h"
#include "cg4block.h"

namespace mdf::detail {
class Sr4Block : public DataListBlock, public ISampleReduction {
 public:
  Sr4Block();
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override;
  void NofSamples(uint64_t nof_samples) override;
  [[nodiscard]] uint64_t NofSamples() const override;

  void Interval(double interval) override;
  [[nodiscard]] double Interval() const override;

  void SyncType(SrSyncType type) override;
  [[nodiscard]] SrSyncType SyncType() const override;

  void Flags(uint8_t flags) override;
  [[nodiscard]] uint8_t Flags() const override;

  [[nodiscard]] const IChannelGroup* ChannelGroup() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE* file) override;

  void ReadData(std::FILE* file) const;  ///< Reads in SR/Rx data
  void ClearData() override;

 protected:
  void GetChannelValueUint( const IChannel& channel, uint64_t sample,
                            uint64_t array_index, SrValue<uint64_t>& value ) const override;
  void GetChannelValueInt( const IChannel& channel, uint64_t sample,
                            uint64_t array_index, SrValue<int64_t>& value ) const override;
  void GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                              uint64_t array_index, SrValue<double>& value ) const override;
 private:
  uint64_t nof_samples_ = 0;
  double interval_ = 0;
  uint8_t type_ = 0;
  uint8_t flags_ = 0;
  /* reserved 6 byte */

  mutable std::vector<uint8_t> data_list_; ///< Typical RD block data


  template <typename T>
  void GetChannelValueT( const IChannel& channel, uint64_t sample,
                               uint64_t array_index, SrValue<T>& value ) const;

};


template<typename T>
void Sr4Block::GetChannelValueT(const IChannel &channel,
                                uint64_t sample,
                                uint64_t array_index,
                                SrValue<T> &value) const {
  const auto* channel_group = ChannelGroup();
  if (data_list_.empty() || channel_group == nullptr) {
    return;
  }

  const auto* cg4 = dynamic_cast<const Cg4Block*>(channel_group);
  if (cg4 == nullptr) {
    return;
  }

  const auto block_size = cg4->NofDataBytes() + cg4->NofInvalidBytes();
  std::vector<uint8_t> record_buffer(block_size, 0);

  if (IsRdBlock()) {
    const auto mean_index = static_cast<int64_t>(sample * 3 * block_size);
    if (static_cast<size_t>(mean_index + block_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + mean_index, block_size, record_buffer.begin() );
      value.MeanValid = channel.GetChannelValue(record_buffer, value.MeanValue, array_index);
    }
    const auto min_index = static_cast<int64_t>(sample * 3 * block_size) + block_size;
    if (static_cast<size_t>(min_index + block_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + min_index, block_size, record_buffer.begin() );
      value.MinValid = channel.GetChannelValue(record_buffer, value.MinValue, array_index);
    }
    const auto max_index = static_cast<int64_t>(sample * 3 * block_size) + (2 * block_size);
    if (static_cast<size_t>(max_index + block_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + max_index, block_size, record_buffer.begin() );
      value.MaxValid = channel.GetChannelValue(record_buffer, value.MaxValue, array_index);
    }
  }
}


}  // namespace mdf::detail
