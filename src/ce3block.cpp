/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ce3block.h"

namespace mdf::detail {
size_t Ce3Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader3(file);
  bytes += ReadNumber(file, type_);
  switch (type_) {
    case 2:bytes += ReadNumber(file, nof_module_);
      bytes += ReadNumber(file, address_);
      bytes += ReadStr(file, description_, 80);
      bytes += ReadStr(file, ecu_, 32);
      break;

    case 19:bytes += ReadNumber(file, message_id_);
      bytes += ReadNumber(file, index_);
      bytes += ReadStr(file, message_, 36);
      bytes += ReadStr(file, sender_, 36);
      break;

    default:break;

  }

  return bytes;
}
}
