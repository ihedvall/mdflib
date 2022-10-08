/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <functional>
#include <sstream>
#include <string>
#if __has_include(<source_location>)
#include <source_location>
#else
#include <experimental/source_location>
#endif
namespace mdf {

#if __has_include(<source_location>)
using Loc = std::source_location;
#else
using Loc = std::experimental::source_location;
#endif

#define MDF_TRACE() \
  MdfLogStream(Loc::current(), MdfLogSeverity::kTrace)  ///< Trace log message
#define MDF_DEBUG() \
  MdfLogStream(Loc::current(), MdfLogSeverity::kDebug)  ///< Debug log message
#define MDF_INFO() \
  MdfLogStream(Loc::current(), MdfLogSeverity::kInfo)  ///< Info log message
#define MDF_ERROR() \
  MdfLogStream(Loc::current(), MdfLogSeverity::kError)  ///< Error log message

///< Defines the log severity level
enum class MdfLogSeverity : uint8_t {
  kTrace = 0,  ///< Trace or listen message
  kDebug,      ///< Debug message
  kInfo,       ///< Informational message
  kNotice,     ///< Notice message. Notify the user.
  kWarning,    ///< Warning message
  kError,      ///< Error message
  kCritical,   ///< Critical message (device error)
  kAlert,      ///< Alert or alarm message
  kEmergency   ///< Fatal error message
};
using MdfLogFunction = std::function<void(
    const Loc& location, MdfLogSeverity severity, const std::string& text)>;

class MdfLogStream : public std::ostringstream {
 public:
  MdfLogStream(const Loc& location, MdfLogSeverity severity);  ///< Constructor
  ~MdfLogStream() override;                                    ///< Destructor

  MdfLogStream() = delete;
  MdfLogStream(const MdfLogStream&) = delete;
  MdfLogStream(MdfLogStream&&) = delete;
  MdfLogStream& operator=(const MdfLogStream&) = delete;
  MdfLogStream& operator=(MdfLogStream&&) = delete;

  static void SetLogFunction(const MdfLogFunction& func);

 protected:
  Loc location_;             ///< File and function location.
  MdfLogSeverity severity_;  ///< Log level of the stream

  virtual void LogString(const Loc& location, MdfLogSeverity severity,
                         const std::string& text);
};

}  // namespace mdf
