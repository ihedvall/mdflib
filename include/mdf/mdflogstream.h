/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>
#include <sstream>
#include <source_location>
namespace mdf {

#define MDF_TRACE() MdfLogStream(std::source_location::current(), MdfLogSeverity::kTrace) ///< Trace log message
#define MDF_DEBUG() MdfLogStream(std::source_location::current(), MdfLogSeverity::kDebug) ///< Debug log message
#define MDF_INFO() MdfLogStream(std::source_location::current(), MdfLogSeverity::kInfo) ///< Info log message
#define MDF_ERROR() MdfLogStream(std::source_location::current(), MdfLogSeverity::kError) ///< Error log message

///< Defines the log severity level
enum class MdfLogSeverity : uint8_t {
  kTrace = 0, ///< Trace or listen message
  kDebug,     ///< Debug message
  kInfo,      ///< Informational message
  kNotice,    ///< Notice message. Notify the user.
  kWarning,   ///< Warning message
  kError,     ///< Error message
  kCritical,  ///< Critical message (device error)
  kAlert,     ///< Alert or alarm message
  kEmergency  ///< Fatal error message
};

class MdfLogStream : public std::ostringstream {

   public:
    MdfLogStream(const std::source_location& location, MdfLogSeverity severity); ///< Constructor
    ~MdfLogStream() override; ///< Destructor

    MdfLogStream() = delete;
    MdfLogStream(const MdfLogStream &) = delete;
    MdfLogStream(MdfLogStream &&) = delete;
    MdfLogStream &operator=(const MdfLogStream &) = delete;
    MdfLogStream &operator=(MdfLogStream &&) = delete;
   protected:
    std::source_location location_;  ///< File and function location.
    MdfLogSeverity severity_;    ///< Log level of the stream

    virtual void LogString(const std::source_location& location, MdfLogSeverity severity, const std::string& text);


};

} // mdf
