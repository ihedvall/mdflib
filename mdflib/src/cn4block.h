/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include <map>

#include "cc4block.h"
#include "datalistblock.h"
#include "md4block.h"
#include "mdf/iattachment.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelhierarchy.h"
#include "si4block.h"
#include "vlsddata.h"

namespace mdf::detail {
class Cg4Block;

class Cn4Block : public DataListBlock, public IChannel {
 public:
  using Cx4List = std::vector<std::unique_ptr<MdfBlock>>;

  Cn4Block();
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }

  void Name(const std::string& name) override;
  [[nodiscard]] std::string Name() const override;

  void DisplayName(const std::string& name) override;
  std::string DisplayName() const override;

  void Description(const std::string& description) override;
  std::string Description() const override;

  void Unit(const std::string& unit) override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] bool IsUnitValid() const override;

  void Flags(uint32_t flags) override;
  uint32_t Flags() const override;

  [[nodiscard]] const IChannelConversion* ChannelConversion() const override;
  [[nodiscard]] IChannelConversion* CreateChannelConversion() override;

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
  [[nodiscard]] const MdfBlock* Find(int64_t index) const override;
  size_t Read(std::FILE* file) override;
  size_t Write(std::FILE* file) override;
  size_t WriteSignalData(std::FILE* file, bool compress);

  void Init(const MdfBlock& id_block) override;

  [[nodiscard]] const Cx4List& Cx4() const { return cx_list_; }

  [[nodiscard]] const Si4Block* Si() const { return si_block_.get(); }
  void AddCc4(std::unique_ptr<Cc4Block>& cc4);
  [[nodiscard]] const Cc4Block* Cc() const { return cc_block_.get(); }
  void ReadData(std::FILE* file) const;  ///< Reads in (VLSD) channel data

  void ClearData() const { data_list_.clear(); }
  const std::vector<uint8_t>& DataList() const { return data_list_; }

  void UpdateDataLink(std::FILE *file, int64_t position);
  [[nodiscard]] int64_t DataLink() const;

  [[nodiscard]] std::vector<int64_t> AtLinkList() const;
  [[nodiscard]] std::vector<int64_t> XAxisLinkList() const;

  void Sync(ChannelSyncType type) override;
  ChannelSyncType Sync() const override;
  void Range(double min, double max) override;
  std::optional<std::pair<double, double>> Range() const override;
  void Limit(double min, double max) override;
  std::optional<std::pair<double, double>> Limit() const override;
  void ExtLimit(double min, double max) override;
  std::optional<std::pair<double, double>> ExtLimit() const override;

  IMetaData *CreateMetaData() override {
    return MdfBlock::CreateMetaData();
  }
  [[nodiscard]] const IMetaData *MetaData() const override {
    return MdfBlock::MetaData();
  }

  [[nodiscard]] const ISourceInformation *SourceInformation() const override;
  [[nodiscard]] ISourceInformation* CreateSourceInformation() override;

  void PrepareForWriting(size_t offset);
  void SetInvalidOffset(size_t bit_offset) {
    invalid_bit_pos_ = static_cast<uint32_t>(bit_offset);
  }
  void SetValid(bool valid) override;
  bool GetValid(const std::vector<uint8_t> &record_buffer) const override;
  void ClearData() override;

  void BitCount(size_t bits) override;
  [[nodiscard]] size_t BitCount() const override;
  void BitOffset(size_t bits) override;
  [[nodiscard]] size_t BitOffset() const override;
  void ByteOffset(size_t bytes) override;
  [[nodiscard]] size_t ByteOffset() const override;
  IChannel* CreateChannelComposition() override;
  std::vector<IChannel*> ChannelCompositions() override;

 protected:

  bool GetTextValue(const std::vector<uint8_t>& record_buffer,
                    std::string& dest) const override;
  bool GetByteArrayValue(const std::vector<uint8_t>& record_buffer,
                    std::vector<uint8_t>& dest) const override;
  void SetTextValue(const std::string &value, bool valid) override;
  void SetByteArray(const std::vector<uint8_t> &value, bool valid) override;

  std::vector<uint8_t>& SampleBuffer() const override;

 private:
  uint8_t type_ = 0;
  uint8_t sync_type_ = 0; ///< Normal channel type
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

  // The block_list_ points to signal data either a SD/DL/DZ block but can also
  // be a reference to an VLSD CG block

  Cx4List cx_list_;
  std::vector<const IAttachment*> attachment_list_;
  ElementLink default_x_;

  // The data_list_ is a temporary buffer that holds
  // uncompressed signal data
  mutable std::vector<uint8_t> data_list_;
  mutable std::map<VlsdData, uint64_t> data_map_; // Data->index map

  const Cg4Block* cg_block_ = nullptr;
};

}  // namespace mdf::detail
