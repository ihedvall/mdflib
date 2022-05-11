/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include "datalistblock.h"
#include "mdf/ichannel.h"
#include "si4block.h"
#include "cc4block.h"
#include "md4block.h"

namespace mdf::detail {
class Cg4Block;

class Cn4Block : public DataListBlock , public IChannel {
 public:
  [[nodiscard]] int64_t Index() const override;

  void Name(const std::string &name) override;
  [[nodiscard]] std::string Name() const override;

  void DisplayName(const std::string &name) override;
  std::string DisplayName() const override;

  void Description(const std::string &description) override;
  std::string Description() const override;

  void Unit(const std::string &unit) override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] bool IsUnitValid() const override;

  [[nodiscard]] const IChannelConversion* ChannelConversion() const override;

  void Type(ChannelType type) override;
  [[nodiscard]] ChannelType Type() const override;

  void DataType(ChannelDataType type) override;
  [[nodiscard]] ChannelDataType DataType() const override;

  void DataBytes(size_t nof_bytes) override;
  [[nodiscard]] size_t DataBytes() const override;

  void SamplingRate(double sampling_rate) override;
  double SamplingRate() const override;

  [[nodiscard]] uint8_t Decimals() const override;

  [[nodiscard]] bool IsDecimalUsed() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] const IBlock* Find(fpos_t index) const override;
  size_t Read(std::FILE *file) override;
  void Init(const IBlock &id_block) override;

  [[nodiscard]] const IBlock* Cx() const {
    return cx_block_.get();
  }

  [[nodiscard]] const Si4Block* Si() const {
    return si_block_.get();
  }

  [[nodiscard]] const Cc4Block* Cc() const {
    return cc_block_.get();
  }
  void ReadData(std::FILE* file) const; ///< Reads in (VLSD) channel data

  void ClearData() const {
    data_list_.clear();
  }
  const std::vector<uint8_t>& DataList() const {
    return data_list_;
  }

  [[nodiscard]] int64_t DataLink() const;
  [[nodiscard]] std::vector<int64_t> AtLinkList() const;
  [[nodiscard]] std::vector<int64_t> XAxisLinkList() const;
 protected:
  size_t BitCount() const; ///< Returns number of bits in value.
  size_t BitOffset() const; ///< Returns bit offset (0..7).
  size_t ByteOffset() const; ///< Returns byte offset in record.
  bool GetTextValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const override;
  std::vector<uint8_t> &SampleBuffer() const override;
 private:
  uint8_t type_ = 0;
  uint8_t sync_type_ = 0;
  uint8_t data_type_ = 0;
  uint8_t bit_offset_ = 0;
  uint32_t byte_offset_ = 0;
  uint32_t bit_count_ = 0;
  uint32_t flags_ = 0;
  uint32_t invalid_bit_pos_ = 0;
  uint8_t precision_ = 0;
  /* 1 byte reserved */
  uint16_t nof_attachments_ = 0;
  double range_min_ = 0;
  double range_max_ = 0;
  double limit_min_ = 0;
  double limit_max_ = 0;
  double limit_ext_min_ = 0;
  double limit_ext_max_ = 0;

  std::string name_;
  std::unique_ptr<Si4Block> si_block_;
  std::unique_ptr<Cc4Block> cc_block_;
  std::unique_ptr<Md4Block> unit_;
  std::unique_ptr<IBlock> cx_block_;

  mutable std::vector<uint8_t> data_list_;
  const Cg4Block* cg_block_ = nullptr;
};

}
