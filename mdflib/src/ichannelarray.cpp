/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelarray.h"
#include <sstream>
namespace mdf {

uint64_t IChannelArray::NofArrayValues() const {
  const auto dimensions = Dimensions();
  uint64_t values = 1;
  for (uint16_t dimension = 0; dimension < dimensions; ++dimension) {
    values *= DimensionSize(dimension);
  }
  return values > 0 ? values : 1;
}

std::string IChannelArray::DimensionAsString() const {
  std::ostringstream out;
  out << "N";
  for (uint16_t dimension = 0; dimension < Dimensions(); ++dimension) {
    out << "[" << DimensionSize(dimension) << "]";
  }
  return out.str();
}

}  // namespace mdf