/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>

#include "isampleobserver.h"
#include "ichannelobserver.h"
#include "ichannelgroup.h"

namespace mdf {
class IDataGroup {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;
  [[nodiscard]] virtual std::string Description() const = 0;
  [[nodiscard]] virtual std::vector<IChannelGroup*> ChannelGroups() const = 0;

  // Implement an observer/subject interface for the reading of samples
  void AttachSampleObserver(ISampleObserver* observer) const;
  void DetachSampleObserver(const ISampleObserver* observer) const;
  void DetachAllSampleObservers() const;
  void NotifySampleObservers(size_t sample, uint64_t record_id, const std::vector<uint8_t>& record) const;

  void ResetSample() const;
  void SetAsRead(bool mark_as_read = true) const {
    mark_as_read_ = mark_as_read;
  }
  [[nodiscard]] bool IsRead() const {
    return mark_as_read_;
  }
 protected:
  mutable std::vector<ISampleObserver*> observer_list;
  virtual ~IDataGroup() = default;

 private:
  mutable bool mark_as_read_ = false;
};

}
