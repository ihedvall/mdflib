/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "cn4block.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdfblock.h"
#include "si4block.h"
#include "sr4block.h"

namespace mdf::detail {

class Cg4Block : public MdfBlock, public IChannelGroup {
 public:
  using Cn4List = std::vector<std::unique_ptr<Cn4Block>>;
  using Sr4List = std::vector<std::unique_ptr<Sr4Block>>;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  const MdfBlock* Find(int64_t index) const override;

  void AddCn4(std::unique_ptr<Cn4Block>& cn3);
  [[nodiscard]] const Cn4List& Cn4() const { return cn_list_; }

  [[nodiscard]] const Sr4List& Sr4() const { return sr_list_; }

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string& name) override;

  [[nodiscard]] std::string Name() const override;

  void Description(const std::string& description) override;
  [[nodiscard]] std::string Description() const override;

  [[nodiscard]] uint64_t NofSamples() const override;
  void NofSamples(uint64_t nof_samples) override;

  [[nodiscard]] uint64_t RecordId() const override;
  void RecordId(uint64_t record_id) override;

  uint16_t Flags() const override;
  void Flags(uint16_t flags) override;

  char16_t PathSeparator() override;
  void PathSeparator(char16_t path_separator) override;

  [[nodiscard]] std::vector<IChannel*> Channels() const override;
  [[nodiscard]] const IChannel* GetXChannel(
      const IChannel& reference) const override;

  const Si4Block* Source() const { return si_block_.get(); }

  size_t Read(std::FILE* file) override;
  void ReadCnList(std::FILE* file);
  void ReadSrList(std::FILE* file);

  size_t ReadDataRecord(std::FILE* file, const IDataGroup& notifier) const;
  std::vector<uint8_t>& SampleBuffer() const { return sample_buffer_; }
  size_t Write(std::FILE* file) override;
  ISourceInformation* CreateSourceInformation() override;
  const ISourceInformation* SourceInformation() const override;

  size_t UpdateCycleCounter(std::FILE *file);
  size_t UpdateVlsdSize(std::FILE *file);

  [[nodiscard]] IMetaData* CreateMetaData() override {
    return MdfBlock::CreateMetaData();
  }
  [[nodiscard]] const IMetaData* MetaData() const override {
    return MdfBlock::MetaData();
  }

 private:
  uint64_t record_id_ = 0;
  uint64_t nof_samples_ = 0;
  uint16_t flags_ = 0;
  uint16_t path_separator_ = u'/';
  /* 4 byte reserved */
  uint32_t nof_data_bytes_ = 0;
  uint32_t nof_invalid_bytes_ = 0;

  std::string acquisition_name_;
  std::unique_ptr<Si4Block> si_block_;
  Cn4List cn_list_;
  Sr4List sr_list_;


};

}  // namespace mdf::detail
