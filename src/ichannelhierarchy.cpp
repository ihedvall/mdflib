/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelhierarchy.h"

namespace mdf {

std::string IChannelHierarchy::TypeToString() const {
  switch (Type()) {
    case ChType::Group:
      return "Group";
    case ChType::Function:
      return "Function";
    case ChType::Structure:
      return "Structure";
    case ChType::MapList:
      return "Map List";
    case ChType::InputVariable:
      return "Input Variables";
    case ChType::OutputVariable:
      return "Output Variables";
    case ChType::LocalVariable:
      return "Local Variables";
    case ChType::CalibrationDefinition:
      return "Calibration Objects Definition";
    case ChType::CalibrationObject:
      return "Calibration Objects Reference";
    default:
      break;
  }
  return {};
}

}  // namespace mdf