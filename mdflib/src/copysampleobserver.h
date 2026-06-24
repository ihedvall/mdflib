/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/samplerecordobserver.h"

namespace mdf {

class CopySampleObserver : public SampleRecordObserver {
public:
  CopySampleObserver() = delete;
  CopySampleObserver(const IDataGroup& source_data_group,
                     const IChannelGroup& source_channel_group,
                     uint64_t base_time,
                     MdfWriter& writer,
                     const IDataGroup& dest_data_group,
                     const IChannelGroup& dest_channel_group);

  void OnSampleRecord() override;
private:
  MdfWriter& writer_;
  const IDataGroup& dest_data_group_;
  const IChannelGroup& dest_channel_group_;

};

}  // namespace mdf

