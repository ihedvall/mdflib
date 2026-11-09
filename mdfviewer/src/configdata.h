/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstdint>

#include <wx/treectrl.h>
#include <wx/string.h>

#include "mdf/iblock.h"
#include "mdf/mdfreader.h"

namespace mdf::viewer {

enum class ConfigPosition:int  {
  kConfig = 0,
  kLayers,
  kLayer,
  kDataFiles,
  kDataFile,
  kDbFiles,
  kDbFile,
  kHexFiles,
  kHexFile,
  kLabFiles,
  kLabFile,
  kAnyFiles,
  kAnyFile,
  kHistories,
  kMeasurements,
  kAttachments,
  kEvents,
  kHierarchies,
  kMdfBlock,
  kSignal
};

class ConfigData : public wxTreeItemData {
public:
  ConfigData() = default;
  bool operator==(const ConfigData& other) const;

  explicit ConfigData(ConfigPosition position, size_t file_index = 0);
  explicit ConfigData(ConfigPosition position, MdfReader* reader);
  explicit ConfigData(MdfReader* reader, int64_t block_index);

  [[nodiscard]] ConfigPosition Position() const { return position_; }
  [[nodiscard]] MdfReader* Reader() const { return reader_; }
  [[nodiscard]] int64_t BlockIndex() const { return block_index_; }
  [[nodiscard]] size_t FileIndex() const { return file_index_; }

private:
  ConfigPosition position_ = ConfigPosition::kMdfBlock;
  MdfReader* reader_ = nullptr;
  int64_t block_index_ = 0;
  size_t file_index_ = 0;
};

}  // namespace mdf::viewer


