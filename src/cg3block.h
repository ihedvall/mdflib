
#pragma once
#include <string>
#include <memory>
#include <vector>
#include "iblock.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "tx3block.h"
#include "cn3block.h"
#include "sr3block.h"
namespace mdf::detail {
class Cg3Block : public IBlock , public IChannelGroup {
 public:
  using Cn3List = std::vector<std::unique_ptr<Cn3Block>>;
  using Sr3List = std::vector<std::unique_ptr<Sr3Block>>;

  [[nodiscard]] int64_t Index() const override;

  void Name(const std::string &name) override;
  [[nodiscard]] std::string Name() const override;

  void Description(const std::string &description) override;
  [[nodiscard]] std::string Description() const override;

  [[nodiscard]] uint64_t NofSamples() const override;
  void NofSamples(uint64_t nof_samples) override;
  void RecordId(uint64_t record_id);
  [[nodiscard]] uint64_t RecordId() const override;

  [[nodiscard]] std::vector<IChannel *> Channels() const override;
  [[nodiscard]] const IChannel* GetXChannel(const IChannel& reference) const override;

  [[nodiscard]] std::string Comment() const override;
  const IBlock* Find(fpos_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;
  void ReadCnList(std::FILE* file);
  void ReadSrList(std::FILE* file);

  uint16_t RecordSize() const {
    return size_of_data_record_;
  }

  const Cn3List& Cn3() const {
    return cn_list_;
  }

  const Sr3List& Sr3() const {
    return sr_list_;
  }

  void AddCn3(std::unique_ptr<Cn3Block>& cn3);

  [[nodiscard]] std::vector<uint8_t>& SampleBuffer() const {
    return sample_buffer_;
  }
  size_t ReadDataRecord(std::FILE* file, const IDataGroup& notifier) const;
 private:

  uint16_t record_id_ = 0;
  uint16_t nof_channels_ = 0;
  uint16_t size_of_data_record_ = 0;
  uint32_t nof_records_ = 0;

  std::string comment_;
  Cn3List cn_list_;
  Sr3List sr_list_;



  void PrepareForWriting();
};
}



