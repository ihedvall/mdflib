/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/iheader.h"

namespace mdf {

void IHeader::MeasurementId(const std::string& uuid) {
}

std::string IHeader::MeasurementId() const {
  return {};
}

void IHeader::RecorderId(const std::string &uuid) {
}

std::string IHeader::RecorderId() const {
  return {};
}

void IHeader::RecorderIndex(int64_t index) {
}

int64_t IHeader::RecorderIndex() const {
  return 0;
}

} // end namespace