/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <memory>
#include "datalistblock.h"
#include "mdf/ichannel.h"
#include "tx3block.h"
#include "cc3block.h"
#include "ce3block.h"
#include "cd3block.h"

namespace mdf::detail {
class Cg3Block;

class Cn3Block : public DataListBlock , public IChannel {
 public:
  [[nodiscard]] int64_t Index() const override;

  void Name(const std::string &name) override;
  [[nodiscard]] std::string Name() const override;

  void DisplayName(const std::string &name) override;
  [[nodiscard]] std::string DisplayName() const override;

  void Description(const std::string &description) override;
  [[nodiscard]] std::string Description() const override;

  void Unit(const std::string &unit) override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] bool IsUnitValid() const override;

  void Type(ChannelType type) override;
  [[nodiscard]] ChannelType Type() const override;

  void DataType(ChannelDataType type) override;
  [[nodiscard]] ChannelDataType DataType() const override;

  void DataBytes(size_t nof_bytes) override;
  [[nodiscard]] size_t DataBytes() const override;

  [[nodiscard]] const IChannelConversion* ChannelConversion() const override;

  void SamplingRate(double sampling_rate) override;
  [[nodiscard]] double SamplingRate() const override;

  [[nodiscard]] uint8_t Decimals() const override;
  [[nodiscard]] bool IsDecimalUsed() const override;


  [[nodiscard]] std::string Comment() const override;
  [[nodiscard]] const IBlock* Find(fpos_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  void Init(const IBlock &id_block) override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  void AddCc3(std::unique_ptr<Cc3Block>& cc3);
  void ByteOffset(uint16_t byte_offset);

  [[nodiscard]] Cc3Block* Cc3() const {
    return cc_block_.get();
  }

  [[nodiscard]] Cd3Block* Cd3() const {
    return cd_block_.get();
  }

  [[nodiscard]] Ce3Block* Ce3() const {
    return ce_block_.get();
  }

 protected:
  [[nodiscard]] size_t BitCount() const override; ///< Returns number of bits in value.
  [[nodiscard]] size_t BitOffset() const override; ///< Returns bit offset (0..7).
  [[nodiscard]] size_t ByteOffset() const override; ///< Returns byte offset in record.

  [[nodiscard]] std::vector<uint8_t> &SampleBuffer() const override;

 private:
  uint16_t channel_type_ = 0;
  std::string short_name_;
  std::string description_;
  uint16_t start_offset_ = 0;
  uint16_t nof_bits_ = 0;
  uint16_t signal_type_ = 0;
  bool range_valid_ = false;
  double min_ = 0.0;
  double max_ = 0.0;
  double sample_rate_ = 0.0;
  uint16_t byte_offset_ = 0;

  std::string comment_;
  std::string long_name_;
  std::string display_name_;

  std::unique_ptr<Cc3Block> cc_block_;
  std::unique_ptr<Ce3Block> ce_block_;
  std::unique_ptr<Cd3Block> cd_block_;

  const Cg3Block* cg3_block = nullptr;
};
}


