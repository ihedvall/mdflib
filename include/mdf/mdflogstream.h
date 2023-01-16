/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <sstream>
#include <string>

struct MdfLocation {
  int line = 0;
  int column = 0;
  std::string file;
  std::string function;
};

#include "mdf/mdffactory.h"

namespace mdf {

#define MDF_TRACE() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kTrace)  ///< Trace log message
#define MDF_DEBUG() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kDebug)  ///< Debug log message
#define MDF_INFO() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kInfo)  ///< Info log message
#define MDF_ERROR() \
  MdfLogStream({__LINE__,0,__FILE__,__func__}, \
               MdfLogSeverity::kError)  ///< Error log message

using MdfLogFunction1 = std::function<void(const MdfLocation &location,
  MdfLogSeverity severity, const std::string &text)>;

class MdfLogStream : public std::ostringstream {
 public:
  MdfLogStream(MdfLocation location, MdfLogSeverity severity);  ///< Constructor
  ~MdfLogStream() override;                                    ///< Destructor

  MdfLogStream() = delete;
  MdfLogStream(const MdfLogStream&) = delete;
  MdfLogStream(MdfLogStream&&) = delete;
  MdfLogStream& operator=(const MdfLogStream&) = delete;
  MdfLogStream& operator=(MdfLogStream&&) = delete;

  static void SetLogFunction1(const MdfLogFunction1& func);
  static void SetLogFunction2(const MdfLogFunction2& func);
  
 protected:
  MdfLocation location_;     ///< File and function location.
  MdfLogSeverity severity_;  ///< Log level of the stream

  virtual void LogString(const MdfLocation& location, MdfLogSeverity severity,
                         const std::string& text);
};

}  // namespace mdf
