/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdflogstream.h"

namespace {

mdf::MdfLogFunction LogFunction;

} // end namespace

namespace mdf {

MdfLogStream::MdfLogStream(const Loc &location, MdfLogSeverity severity)
    : location_(location),
      severity_(severity) {
}

MdfLogStream::~MdfLogStream() {
  MdfLogStream::LogString(location_, severity_, str());
}

void MdfLogStream::LogString(const Loc &location, MdfLogSeverity severity, const std::string &text) {
  if (LogFunction) {
    LogFunction(location, severity, text);
  }
}

void MdfLogStream::SetLogFunction(const MdfLogFunction &func) {
  LogFunction = func;
}

} // mdf