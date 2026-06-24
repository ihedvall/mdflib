/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */


#pragma once
#include "mdf/mdftask.h"

namespace mdf {

class SortingTask : public MdfTask {
public:
  SortingTask() = default;
  ~SortingTask() override = default;

  void Run() override;

private:
  uint64_t start_time_ = 0;
  uint64_t sample_time_ = 0;
  ChannelObserverList observer_list_;

  void SortFile();
  void ReadInData(IDataGroup& data_group, const IChannelGroup& channel_group);
  void CopyData(const IChannelGroup& source_cg, const IChannelGroup& dest_cg);
};

}  // namespace mdf

