/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "configdata.h"

namespace mdf::viewer {

bool ConfigData::operator==(const ConfigData& other) const {
  if (position_ != other.position_) {
    return false;
  }
  if (block_index_ != other.block_index_) {
    return false;
  }
  if (reader_ != other.reader_) {
    return false;
  }
  return file_index_ == other.file_index_;
}

ConfigData::ConfigData(ConfigPosition position, size_t file_index)
  : wxTreeItemData(),
    position_(position),
    file_index_(file_index) {
}

ConfigData::ConfigData(ConfigPosition position, MdfReader* reader)
  : wxTreeItemData(),
    position_(position),
    reader_ (reader) {

}

ConfigData::ConfigData(MdfReader* reader, int64_t block_index )
  : wxTreeItemData(),
    reader_(reader),
    block_index_(block_index) {
}

} // mdf::viewer