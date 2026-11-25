/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/samplerecordobserver.h"

#include <cmath>

namespace mdf {

SampleRecordObserver::SampleRecordObserver(const IDataGroup& data_group,
                                           const IChannelGroup& channel_group,
                                           uint64_t base_time)
: ISampleObserver(data_group),
  channel_group_(channel_group),
  base_time_(base_time),
  time_channel_(channel_group.GetMasterTimeChannel()) {

  record_id_list_.clear();
  record_id_list_.insert(channel_group.RecordId());
  FindVlsdRecord(channel_group);
}

bool SampleRecordObserver::OnSample(uint64_t sample, uint64_t record_id,
                              const std::vector<uint8_t>& record) {
  if (DataGroup() == nullptr ||
      record_id_list_.find(record_id) == record_id_list_.end()) {
    // Not interested in this record.
    return true;
  }

  const auto current_sample_index = static_cast<int64_t>(sample);
  if (current_sample_index > current_sample_index_) {
    last_sample_.Clear();
    current_sample_index_ = current_sample_index;
  }

  if (record_id == channel_group_.RecordId()) {
    last_sample_.record_id = record_id;
    last_sample_.record_buffer = record;
    last_sample_.timestamp = base_time_;
    if (time_channel_ != nullptr) {
      double rel_time = time_channel_->GetTimestamp(GetSampleIndex(), record);
      // Round, a truncated time can move an integer master one tick back
      last_sample_.timestamp += std::llround(rel_time * 1'000'000'000);
    }
    sample_time_ = last_sample_.timestamp;
    if (!sd_vlsd_list_.empty()) {
      // VLSD data is stored in a channels signa data block.
      // This handle by the channel object
      if (const IChannel* vlsd_channel = sd_vlsd_list_.front();
          vlsd_channel != nullptr) {
        last_sample_.vlsd_data = true;
        // Raw bytes, GetChannelValue() would convert text to UTF-8
        vlsd_channel->GetByteArrayValue(record, last_sample_.vlsd_buffer);
      }
    }
  } else {
    last_sample_.vlsd_data = true;
    last_sample_.vlsd_buffer = record;
  }

  if (!last_sample_.record_buffer.empty()) {
    HandleSample();
  }
  return true;
}

uint64_t SampleRecordObserver::GetSampleIndex() const {
  if (current_sample_index_ < 0) {
    return 0;
  }
  return static_cast<uint64_t>(current_sample_index_);
}

void SampleRecordObserver::GetSampleRecord(SampleRecord& sample_record) {
  sample_record = std::move(last_sample_);
}

void SampleRecordObserver::HandleSample() {
  switch (channel_group_.StorageType()) {

    case MdfStorageType::VlsdStorage:
      // Wait for the VLSD CG record callback.
      // It received in the callback before this callback. Note that
      // this assumes that the logger uses this one-to-one saving
      // policy.
      if (last_sample_.vlsd_data) {
        OnSampleRecord();
        last_sample_.Clear();
      }
      break;

    case MdfStorageType::MlsdStorage:
    case MdfStorageType::FixedLengthStorage:
      // The VLSD bytes is stored in the data bytes channel.
      // Its length is stored in another channel (have to be data length).
      // There is some bugs for some loggers to not correctly point out the
      // correct data length channel.
      OnSampleRecord();
      last_sample_.Clear();
      break;

    default:
      break;
  }
}

} // mdf