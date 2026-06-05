/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdflogstream.h"

#include <iostream>
#include <array>
#include <string_view>
#include <vector>

#include "mdf/mdfhelper.h"

namespace {

std::vector<mdf::MdfLogFunction1> kLogFunction1List;
std::vector<mdf::MdfLogFunction2> kLogFunction2List;

constexpr std::array<std::string_view, 9> kSeverityList = {"Trace", "Debug",
                                                       "Info", "Notice",
                                                       "Warning", "Error",
                                                       "Critical","Alert",
                                                       "Emergency" };
mdf::MdfLogSeverity kLogLevel = mdf::MdfLogSeverity::kTrace;
}  // end namespace

namespace mdf {

MdfLogStream::MdfLogStream(MdfLocation location, MdfLogSeverity severity)
    : location_(std::move(location)), severity_(severity) {}

MdfLogStream::~MdfLogStream() {
  MdfLogStream::LogString(location_, severity_, str());
}

void MdfLogStream::LogString(const MdfLocation &location, MdfLogSeverity severity,
                             const std::string &text) {
  for (const auto &func1 : kLogFunction1List) {
    if (func1) {
      func1(location, severity, text);
    }
  }
  for (const auto &func2 : kLogFunction2List) {
    if (func2) {
      std::ostringstream func;
      func << location.file << ":" << location.function;
      func2(severity, func.str(), text);
    }
  }

}

void MdfLogStream::SetLogFunction1(const MdfLogFunction1 &func) {
  kLogFunction1List.emplace_back(func);
}

void MdfLogStream::SetLogFunction2(const MdfLogFunction2 &func) {
  kLogFunction2List.emplace_back(func);
}

void MdfLogStream::ResetLogFunction() {
  kLogFunction1List.clear();
  kLogFunction2List.clear();
}

void MdfLogStream::LogToConsole(const MdfLocation& location,
                                   MdfLogSeverity severity,
                                   const std::string& text) {
  if (severity < kLogLevel) {
    return;
  }
  try {
    const uint64_t now = MdfHelper::NowNs();
    const std::string now_string = MdfHelper::NsToLocalIsoTime(now);
    std::cout << now_string << " ";
    if (const auto index = static_cast<size_t>(severity);
        index < kSeverityList.size()) {
      std::cout << "[" << kSeverityList[index] << "] ";
    }

    std::cout << text << " ";
    std::cout << "(" << location.file << ":"
                << location.function << " C:" << location.column
                << " L:" << location.line << ")";

    std::cout << std::endl;
  } catch (const std::exception&) {

  }
}

void MdfLogStream::SetLogLevel(MdfLogSeverity severity) {
  kLogLevel = severity;
}

}  // namespace mdf