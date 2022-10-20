/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>

#include "ichannelgroup.h"
#include "isampleobserver.h"

namespace mdf {

class IMetaData;

class IDataGroup {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void Description(const std::string& desc);
  [[nodiscard]] virtual std::string Description() const;

  virtual void RecordIdSize(uint8_t id_size);
  [[nodiscard]] virtual uint8_t RecordIdSize() const;

  [[nodiscard]] virtual std::vector<IChannelGroup*> ChannelGroups() const = 0;
  [[nodiscard]] virtual IChannelGroup* CreateChannelGroup() = 0;

  [[nodiscard]] virtual IMetaData* MetaData();
  [[nodiscard]] virtual const IMetaData* MetaData() const;

  // Implement an observer/subject interface for the reading of samples
  void AttachSampleObserver(ISampleObserver* observer) const;
  void DetachSampleObserver(const ISampleObserver* observer) const;
  void DetachAllSampleObservers() const;
  void NotifySampleObservers(size_t sample, uint64_t record_id,
                             const std::vector<uint8_t>& record) const;

  void ResetSample() const;
  void SetAsRead(bool mark_as_read = true) const {
    mark_as_read_ = mark_as_read;
  }
  [[nodiscard]] bool IsRead() const { return mark_as_read_; }

  [[nodiscard]] virtual const IChannelGroup *FindParentChannelGroup(
      const IChannel &channel) const = 0;

 protected:
  mutable std::vector<ISampleObserver*> observer_list;
  virtual ~IDataGroup() = default;

 private:
  mutable bool mark_as_read_ = false;
};

}  // namespace mdf
