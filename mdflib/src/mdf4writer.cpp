/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf4writer.h"
#include <ctime>
#include "mdf/mdflogstream.h"
#include "mdf4file.h"
#include "platform.h"
#include "dt4block.h"
#include "hl4block.h"
#include "dl4block.h"
#include "dz4block.h"
#include "dg4block.h"
namespace mdf::detail {

Mdf4Writer::~Mdf4Writer() { StopWorkThread(); }

IChannelConversion* Mdf4Writer::CreateChannelConversion(IChannel* parent) {
  auto* cn4 = dynamic_cast<Cn4Block*>(parent);
  IChannelConversion* cc = nullptr;
  if (cn4 != nullptr) {
    auto cc4 = std::make_unique<Cc4Block>();
    cc4->Init(*cn4);
    cn4->AddCc4(cc4);
    cc = const_cast<Cc4Block*>(cn4->Cc());
  }
  return cc;
}

void Mdf4Writer::CreateMdfFile() {
  auto mdf4 = std::make_unique<Mdf4File>();
  mdf_file_ = std::move(mdf4);
}

void Mdf4Writer::SetLastPosition(std::FILE* file) {
  Platform::fseek64(file, 0, SEEK_END);

  auto* header = Header();
  if (header == nullptr) {
    return;
  }
  auto* last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    return;
  }

  if (dg4->Link(2) > 0) {
    return;
  }

  dg4->SetLastFilePosition(file);
  auto position = GetFilePosition(file);
  dg4->UpdateLink(file, 2, position);
  dg4->SetLastFilePosition(file);
}

bool Mdf4Writer::PrepareForWriting() {

  auto *header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No header  found. Invalid use of the function.";
    return false;
  }

  // Only the last DG block is updated. So go to the last
  // DG block and add an uncompressed DT block to the DG block
  // or an HL/DL/DZ thing for compressed data.
  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return true;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    MDF_ERROR() << "Invalid DG block type detected.";
    return false;
  }
  if (CompressData()) {
    auto hl4 = std::make_unique<Hl4Block>();
    hl4->Init(*dg4);
  } else {
    // The data block list should include a DT block that we later will append
    // samples (buffers) to.
    auto& block_list = dg4->DataBlockList();
    auto dt4 = std::make_unique<Dt4Block>();
    dt4->Init(*dg4);
    block_list.push_back(std::move(dt4));
  }

  // Size the sample buffers for each CG block
  auto cg_list = dg4->ChannelGroups();
  for (auto* group : cg_list) {
    if (group == nullptr) {
      continue;
    }
    auto* cg4 = dynamic_cast<Cg4Block*>(group);
    if (cg4 == nullptr) {
      continue;
    }
    cg4->PrepareForWriting();
  }

  return true;
}

void Mdf4Writer::SaveQueue(std::unique_lock<std::mutex>& lock) {
  if (CompressData()) {
    SaveQueueCompressed(lock);
    return;
  }

  // Save uncompressed data in last DG/DT block
  auto *header = Header();
  if (header == nullptr) {
    return;
  }

  // Only the last DT block is updated. So go to the last DT
  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    return;
  }
  auto& block_list = dg4->DataBlockList();
  if (block_list.empty()) {
    return;
  }
  auto* dt4 = dynamic_cast<Dt4Block*>(block_list.back().get());
  if (dt4 == nullptr) {
    return;
  }
  const auto data_position = dt4->DataPosition();
  if (data_position <= 0) {
    return;
  }
  lock.unlock();

  std::FILE* file = nullptr;
  Platform::fileopen(&file, filename_.c_str(), "r+b");
  if (file == nullptr) {
    lock.lock();
    return;
  }

  SetLastPosition(file);

  lock.lock();

  TrimQueue();

  while (!sample_queue_.empty()) {
    // Write a sample last to file
    const auto sample = sample_queue_.front();
    sample_queue_.pop_front();

    if (stop_time_ > 0 && sample.timestamp > stop_time_) {
      break;
    }
    lock.unlock();

    const auto id_size = dg4->RecordIdSize();
    switch (id_size) {
      case 0:
        // No Record ID to store
        break;

      case 1: {
        const auto id = static_cast<uint8_t>(sample.record_id);
        dg4->WriteNumber(file, id);
        break;
      }

      case 2: {
        const auto id = static_cast<uint16_t>(sample.record_id);
        dg4->WriteNumber(file, id);
        break;
      }

      case 4: {
        const auto id = static_cast<uint32_t>(sample.record_id);
        dg4->WriteNumber(file, id);
        break;
      }

      default: {
        const auto id = static_cast<uint64_t>(sample.record_id);
        dg4->WriteNumber(file, id);
        break;
      }

    }

    fwrite(sample.record_buffer.data(), 1, sample.record_buffer.size(), file);

    IncrementNofSamples(sample.record_id);
    lock.lock();
  }

  lock.unlock();
  const auto last_position = GetFilePosition(file);
  uint64_t block_length = 24 + (last_position - data_position);
  dt4->UpdateBlockSize(file, block_length);
  dg4->Write(file); // Flush out data
  fclose(file);
  lock.lock();

}

void Mdf4Writer::CleanQueue(std::unique_lock<std::mutex>& lock) {
  if (CompressData()) {
    CleanQueueCompressed(lock);
    return;
  }
  SaveQueue(lock);
}

void Mdf4Writer::SaveQueueCompressed(std::unique_lock<std::mutex>& lock) {
  TrimQueue();

  const auto nof_dz = CalculateNofDzBlocks();
  if (nof_dz < 2) {
    // Only save full 4MB DZ blocks
    return;
  }
  constexpr time_t min10 = 10 * 60;
  const time_t now = time(nullptr);
  if (start_time_== 0) {
    save_timer_ = 0;
  } else if (save_timer_ == 0) {
    save_timer_ = now + min10;
  }

  if (nof_dz < 5 && now < save_timer_) {
    // If less than 5 * 4MB within 10 minutes, wait with saving to file
    return;
  }
  // Save all blocks to file
  save_timer_ = now + min10;
  CleanQueueCompressed(lock);
}

void Mdf4Writer::CleanQueueCompressed(std::unique_lock<std::mutex>& lock) {
  // Save compressed data in last DG block by appending HL/DL and DZ/DT blocks
  constexpr size_t buffer_max = 4'000'000;
  if (sample_queue_.empty()) {
    // Nothing to save to the file.
    return;
  }

  auto *header = Header();
  if (header == nullptr) {
    return;
  }

  // Only the last DT block is updated. So go to the last DT
  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    return;
  }

  if (dg4->DataBlockList().empty()) {
    auto& hl4_list = dg4->DataBlockList();
    // Add a HL4 block
    auto hl_block = std::make_unique<Hl4Block>();
    hl_block->Flags(0);
    hl_block->Type(Hl4ZipType::Deflate);
    hl4_list.push_back(std::move(hl_block));
  }
  auto* hl4 = dynamic_cast<Hl4Block*>(dg4->DataBlockList().back().get());
  if (hl4 == nullptr) {
    return;
  }

  // Open the file for writing
  lock.unlock();

  std::FILE* file = nullptr;
  Platform::fileopen(&file, filename_.c_str(), "r+b");
  if (file == nullptr) {
    lock.lock();
    return;
  }

  SetLastPosition(file);

  lock.lock();
  TrimQueue();

  std::vector<uint8_t> buffer;
  buffer.reserve(4'000'000);

  // Create DL block
  auto dl4 = std::make_unique<Dl4Block>();
  dl4->Flags(0);

  size_t dz_count = 0;
  dl4->Offset(dz_count, offset_);

  while (!sample_queue_.empty()) {
    const auto sample = sample_queue_.front();
    sample_queue_.pop_front();

    if (stop_time_ > 0 && sample.timestamp > stop_time_) {
      // No more data.
      break;
    }
    const size_t max_index = sample.record_buffer.size()
                             + dg4->RecordIdSize()
                             + buffer.size();
    if (max_index >= buffer_max) {

      // Purge the buffer to a DZ block and add it to the last DL block
      auto dz4 = std::make_unique<Dz4Block>();
      dz4->OrigBlockType("DT");
      dz4->Type(Dz4ZipType::Deflate);
      dz4->Data(buffer);
      dl4->Offset(dz_count, offset_);
      ++dz_count;
      offset_ += buffer.size();
      dl4->DataBlockList().push_back(std::move(dz4));


      buffer.clear();
      buffer.reserve(buffer_max);
    }

    const auto id_size = dg4->RecordIdSize();
    switch (id_size) {
      case 0:
        // No Record ID to store
        break;

      case 1: {
        const auto id = static_cast<uint8_t>(sample.record_id);
        LittleBuffer<uint8_t> temp(id);
        buffer.insert(buffer.end(), temp.cbegin(), temp.cend());
        break;
      }

      case 2: {
        const auto id = static_cast<uint16_t>(sample.record_id);
        LittleBuffer<uint16_t> temp(id);
        buffer.insert(buffer.end(), temp.cbegin(), temp.cend());
        break;
      }

      case 4: {
        const auto id = static_cast<uint32_t>(sample.record_id);
        LittleBuffer<uint32_t> temp(id);
        buffer.insert(buffer.end(), temp.cbegin(), temp.cend());
        break;
      }

      default: {
        const auto id = static_cast<uint64_t>(sample.record_id);
        LittleBuffer<uint64_t> temp(id);
        buffer.insert(buffer.end(), temp.cbegin(), temp.cend());
        break;
      }

    }
    buffer.insert(buffer.end(), sample.record_buffer.cbegin(),
                  sample.record_buffer.cend());
    IncrementNofSamples(sample.record_id);
  }


  if (!buffer.empty()) {
    if (buffer.size() > 100) {
      auto dz4 = std::make_unique<Dz4Block>();
      dz4->OrigBlockType("DT");
      dz4->Type(Dz4ZipType::Deflate);
      dz4->Data(buffer);
      auto& block_list = dl4->DataBlockList();
      block_list.push_back(std::move(dz4));
    } else {
      auto dt4 = std::make_unique<Dt4Block>();
      dt4->Data(buffer);
      auto& block_list = dl4->DataBlockList();
      block_list.push_back(std::move(dt4));
    }
    dl4->Offset(dz_count, offset_);
    ++dz_count;
    offset_ += buffer.size();
  }
  hl4->DataBlockList().push_back(std::move(dl4));

  lock.unlock();
  dg4->Write(file); // Flush out data
  fclose(file);
  lock.lock();
  hl4->ClearData(); // Remove temp data
}

void Mdf4Writer::SetDataPosition(std::FILE* file) {
  if (CompressData()) {
    return;
  }
  auto *header = Header();
  if (header == nullptr) {
    return;
  }

  // Only the last DT block is updated. So go to the last DT
  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    return;
  }
  auto& block_list = dg4->DataBlockList();
  if (block_list.empty()) {
    return;
  }
  auto* dt4 = dynamic_cast<Dt4Block*>(block_list.back().get());
  if (dt4 == nullptr) {
    return;
  }
  SetLastPosition(file);
  const auto data_position = GetFilePosition(file);
  dt4->DataPosition(data_position);
}

size_t Mdf4Writer::CalculateNofDzBlocks() {
  const auto *header = Header();
  if (header == nullptr) {
    return 0;
  }

  // Only the last DG block is updated. So go to the last DT
  const auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return 0;
  }
  const auto* dg4 = dynamic_cast<const Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    return 0;
  }

  uint64_t nof_bytes = 0;
  const auto& cg4_list = dg4->Cg4();
  for (const auto& cg4 : cg4_list) {
    if (!cg4 || cg4->NofSamples() == 0) {
      continue;
    }
    const auto& sample_buffer = cg4->SampleBuffer();
    const uint64_t record_size  = dg4->RecordIdSize() + sample_buffer.size();
    nof_bytes += cg4->NofSamples() * record_size;
  }
  return (nof_bytes / 4'000'000) + 1;
}

bool Mdf4Writer::InitMeasurement() {
  offset_ = 0;
  return MdfWriter::InitMeasurement();
}

bool Mdf4Writer::WriteSignalData(std::FILE* file) {
  if (file == nullptr) {
    MDF_ERROR() << "File is not opened. File: " << Name();
    return false;
  }

  const auto *header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No header block found. File: " << Name();
    return false;
  }

  // Only the last DG block is updated. So go to the last DT
  const auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return true;
  }

  auto cg_list = last_dg->ChannelGroups();
  for (auto* group : cg_list) {
    if (group == nullptr) {
      continue;
    }
    auto cn_list = group->Channels();
    for (auto* channel : cn_list) {
      if (channel == nullptr) {
        continue;
      }
      auto* cn4 = dynamic_cast<Cn4Block*>(channel);
      if (cn4 == nullptr) {
        continue;
      }
      cn4->WriteSignalData(file, CompressData());
      cn4->ClearData();
    }
  }
  return true;
}

}  // namespace mdf::detail