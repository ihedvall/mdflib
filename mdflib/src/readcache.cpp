/*
 * Copyright 2024 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "readcache.h"
#include <iostream>

namespace mdf::detail {

ReadCache::ReadCache( MdfBlock* block, FILE* file)
: file_(file)
{
  data_list_ = dynamic_cast<DataListBlock*>(block);
  data_block_ = dynamic_cast<DataBlock*>(block);
  dg4_block_ = dynamic_cast<Dg4Block*>(block);

  if (dg4_block_ != nullptr) {
    const auto cg_list = dg4_block_->ChannelGroups();
    for (const auto* channel_group : cg_list) {
      if (channel_group != nullptr && dg4_block_->IsSubscribingOnRecord(channel_group->RecordId())) {
        record_id_list_.insert(channel_group->RecordId());
      }
    }
  }

  if (data_list_ != nullptr) {
    max_data_count_ = data_list_->DataSize();
  } else if (data_block_ != nullptr) {
    max_data_count_ = data_block_->DataSize();
  }

  // Make at list of data blocks that simplify the stepping of data blocks,
  // The block_index
  if (data_list_ != nullptr)  {
    data_list_->GetDataBlockList(block_list_);
  } else if (data_block_ != nullptr) {
    block_list_.push_back(data_block_);
  }
}

bool ReadCache::ParseRecord() {

  if (dg4_block_ == nullptr || dg4_block_->Cg4().empty() ) {
    return false;
  }
  if (data_count_ >= max_data_count_ || block_list_.empty()) {
    return false;
  }

  try {
    const auto record_id = ParseRecordId();
    const auto *channel_group = dg4_block_->FindCgRecordId(record_id);
    if (channel_group == nullptr) {
      throw std::runtime_error("No channel group found.");
    }
    if (channel_group->Flags() & CgFlag::VlsdChannel) {
      // This is normally used for string and the CG block only include one signal
      std::vector<uint8_t> temp(4, 0);
      GetArray(temp);
      const LittleBuffer<uint32_t> length(temp, 0);

      if (record_id_list_.find(record_id) == record_id_list_.cend()) {
        SkipBytes(length.value());
        const size_t sample = channel_group->Sample();
        if (sample < channel_group->NofSamples()) {
          channel_group->IncrementSample();
        }
      } else {
        std::vector<uint8_t> record(length.value(), 0);
        GetArray(record);
        const size_t sample = channel_group->Sample();
        if (sample < channel_group->NofSamples()) {
          dg4_block_->NotifySampleObservers(sample, channel_group->RecordId(), record);
          channel_group->IncrementSample();
        };
      }

    } else {
      const size_t record_size = channel_group->NofDataBytes()
                                 + channel_group->NofInvalidBytes();
      // Normal fixed length records
      if (record_id_list_.find(record_id) == record_id_list_.cend()) {
        SkipBytes(record_size);
        const size_t sample = channel_group->Sample();
        if (sample < channel_group->NofSamples()) {
          channel_group->IncrementSample();
        }
      } else {
        std::vector<uint8_t> record(record_size, 0);
        GetArray(record);
        const size_t sample = channel_group->Sample();
        if (sample < channel_group->NofSamples()) {
          dg4_block_->NotifySampleObservers(sample, channel_group->RecordId(), record);
          channel_group->IncrementSample();
        }
      }
    }
  } catch (const std::exception &) {
    return false;
  }

  return true;
}

bool ReadCache::ParseSignalData() {
  if (data_count_ >= max_data_count_ || block_list_.empty()) {
    return false;
  }

  while (data_count_ < max_data_count_ ) {
    bool skip = false;
    try {

      std::vector<uint8_t> temp(4, 0);
      GetArray(temp);
      const LittleBuffer<uint32_t> length(temp, 0);

      if (!offset_filter_.empty() &&
           offset_filter_.find(offset_) == offset_filter_.cend()) {
        SkipBytes(length.value());
        skip = true;
        offset_ += 4 + length.value();
      } else {
        std::vector<uint8_t> raw_data(length.value());
        GetArray(raw_data);
        if (callback_) {
          callback_(offset_, raw_data);
        }
        offset_ += 4 + length.value();
      }
    } catch (const std::exception &) {
      return false;
    }
    if (!skip) {
      break;
    }
  }
  return data_count_ <= max_data_count_;

}
bool ReadCache::ParseVlsdCgData() {

  if (dg4_block_ == nullptr || dg4_block_->Cg4().empty() ) {
    return false;
  }
  if (data_count_ >= max_data_count_ || block_list_.empty()) {
    return false;
  }

  while (data_count_ < max_data_count_ ) {
    bool skip = false;
    try {
      const auto record_id = ParseRecordId();
      const auto *channel_group = dg4_block_->FindCgRecordId(record_id);
      if (channel_group == nullptr) {
        throw std::runtime_error("No channel group found.");
      }
      if (channel_group->Flags() & CgFlag::VlsdChannel) {
        // This is normally used for string and the CG block only include one signal
        std::vector<uint8_t> temp(4, 0);
        GetArray(temp);
        const LittleBuffer<uint32_t> length(temp, 0);

        if (record_id_list_.find(record_id) == record_id_list_.cend()) {
          SkipBytes(length.value());
          skip = true; // Do not return
        } else if (!offset_filter_.empty() &&
                   offset_filter_.find(offset_) == offset_filter_.cend()) {
          SkipBytes(length.value());
          offset_ += 4 + length.value();
          skip = true;
        } else {
          std::vector<uint8_t> raw_data(length.value());
          GetArray(raw_data);
          if (callback_) {
            callback_(offset_, raw_data);
          }
          offset_ += 4 + length.value();
        }
      } else {
        const size_t record_size = channel_group->NofDataBytes()
                                   + channel_group->NofInvalidBytes();
        SkipBytes(record_size);
        skip = true;
      }
    } catch (const std::exception &) {
      return false;
    }
    if (!skip) {
      break;
    }
  }

  return data_count_ <= max_data_count_;
}

bool ReadCache::GetNextByte(uint8_t &input) {
  if (file_ == nullptr) {
    return false;
  }
  if ( file_index_ < data_size_) {
    if (file_buffer_.empty()) {
      // Read direct from file
      const auto nof_bytes = fread(&input,1,1,file_);
      if (nof_bytes != 1) {
        return false;
      }
    } else {
      // Read from buffer in case of DZ block
      input = file_buffer_[file_index_];
    }
    ++file_index_;
    ++data_count_;
    return true;
  }

  if (block_index_ >= block_list_.size()) {
    return false;
  }

  auto* current_block = block_list_[block_index_];
  ++block_index_;
  if (current_block == nullptr) {
    return false;
  }
  file_index_ = 0;
  data_size_ = current_block->DataSize();
  if (current_block->BlockType() == "DZ") {
    // Need a temp buffer in between
    try {
      file_buffer_.resize(data_size_);
    } catch (const std::exception&) {
       return false;
    }
    size_t temp_index = 0;
    current_block->CopyDataToBuffer(file_,file_buffer_, temp_index);
  } else {
    // Read from file directly
    SetFilePosition(file_, current_block->DataPosition());
    file_buffer_.clear(); // Indicate that we read from file directly
  }
    // Read  in the data form file to
  if (file_index_ >= data_size_) {
    return false;
  }
  if (file_buffer_.empty()) {
    const auto reads = fread(&input, 1, 1, file_);
    if (reads != 1) {
      return false;
    }
  } else {
    input = file_buffer_[file_index_];
  }
  ++file_index_;
  ++data_count_;
  return true;
}

uint64_t ReadCache::ParseRecordId() {
  if (dg4_block_ == nullptr) {
    throw std::runtime_error("Not a DG4 block");
  }

  const auto id_size = dg4_block_->RecordIdSize();
  switch (id_size) {
    case 0: {
      // Only one CG group
      const auto& cg_list = dg4_block_->Cg4();
      return cg_list.empty() ? 0 : cg_list[0]->RecordId();
    }

    case 1: {
      std::vector<uint8_t> temp(1);
      GetArray(temp);
      return temp[0];
    }

    case 2: {
      std::vector<uint8_t> temp(2,0);
      GetArray(temp);
      const LittleBuffer<uint16_t> record_id(temp,0);
      return record_id.value();
    }

    case 4: {
      std::vector<uint8_t> temp(4,0);
      GetArray(temp);
      const LittleBuffer<uint32_t> record_id(temp,0);
      return record_id.value();
    }

    case 8: {
      std::vector<uint8_t> temp(8,0);
      GetArray(temp);
      const LittleBuffer<uint64_t> record_id(temp,0);
      return record_id.value();
    }

    default:
      break;

  }
  throw std::runtime_error("Invalid record ID size.");
}


void ReadCache::GetArray(std::vector<uint8_t> &buffer) {
  const auto nof_bytes = buffer.size();
  if (file_index_ + nof_bytes <= data_size_) {
    if (file_buffer_.empty()) {
      const auto bytes = fread(buffer.data(), 1, nof_bytes, file_);
      if (bytes != nof_bytes) {
        throw std::runtime_error("End of file detected.");
      }
    } else {
      std::copy_n(file_buffer_.cbegin() + static_cast<int>(file_index_),
                  nof_bytes,buffer.begin());
    }
    file_index_ += nof_bytes;
    data_count_ += nof_bytes;
    return;
  }
  for (auto& input : buffer) {
    if (!GetNextByte(input)){
      throw std::runtime_error("End of file detected.");
    }
  }
}

void ReadCache::SkipBytes(size_t nof_skip) {
  if (file_index_ + nof_skip <= data_size_) {
    if (file_buffer_.empty()) {
      const auto bytes = StepFilePosition(file_, nof_skip);
      if (bytes != nof_skip) {
        throw std::runtime_error("End of file detected.");
      }
    }
    file_index_ += nof_skip;
    data_count_ += nof_skip;
    return;
  }
  for (size_t skip = 0; skip < nof_skip; ++skip) {
    if (!SkipByte()){
      throw std::runtime_error("End of file detected.");
    }
  }
}

bool ReadCache::SkipByte() {
  if (file_ == nullptr) {
    return false;
  }
  if ( file_index_ < data_size_) {
    if (file_buffer_.empty()) {
      const auto nof_bytes = StepFilePosition(file_, 1);
      if (nof_bytes != 1) {
        return false;
      }
    }
    ++file_index_;
    ++data_count_;
    return true;
  }
  file_index_ = 0;
  data_size_ = 0;
  if (block_index_ >= block_list_.size()) {
    return false;
  }

  auto* current_block = block_list_[block_index_];
  ++block_index_;
  if (current_block == nullptr) {
    return false;
  }
  data_size_ = current_block->DataSize();
  if (current_block->BlockType() == "DZ") {
    try {
      file_buffer_.resize(data_size_);
    } catch (const std::exception&) {
      return false;
    }
  } else {
    file_buffer_.clear();
    SetFilePosition(file_, current_block->DataPosition());
  }

  if (file_index_ >= data_size_) {
    return false;
  }
  if (file_buffer_.empty()) {
    const auto reads = StepFilePosition(file_, 1);
    if (reads != 1) {
      return false;
    }
  }
  ++file_index_;
  ++data_count_;
  return true;
}

void ReadCache::SetOffsetFilter(const std::vector<uint64_t> &offset_list) {
  offset_filter_.clear();
  for (const auto offset : offset_list ) {
    offset_filter_.insert(offset);
  }
}

void ReadCache::SetCallback(const std::function<void(uint64_t, const std::vector<uint8_t> &)> &callback) {
  callback_ = callback;
}


} // Rnd namespace mdf::detail