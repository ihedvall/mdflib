/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <functional>

namespace mdf {

enum class MdfWriterType : int { Mdf3Basic = 0, Mdf4Basic = 1 };

enum class MdfFileType : int { Mdf3FileType = 0, Mdf4FileType = 1 };

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

using MdfLogFunction2 = std::function<void(MdfLogSeverity severity,
  const std::string& function, const std::string& text)>;
class MdfWriter;
class MdfFile;
class MdfFactory {
 public:
  static std::unique_ptr<MdfWriter> CreateMdfWriter(MdfWriterType type);
  static std::unique_ptr<MdfFile> CreateMdfFile(MdfFileType type);
  static void SetLogFunction2(const MdfLogFunction2& func);
};

}  // namespace mdf