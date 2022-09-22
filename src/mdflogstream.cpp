/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdflogstream.h"

namespace mdf {
MdfLogStream::MdfLogStream(const std::source_location &location, MdfLogSeverity severity)
    : location_(location),
      severity_(severity) {
}

MdfLogStream::~MdfLogStream() {
  MdfLogStream::LogString(location_, severity_, str());
}

void MdfLogStream::LogString(const std::source_location &location, MdfLogSeverity severity, const std::string &text) {

}

} // mdf