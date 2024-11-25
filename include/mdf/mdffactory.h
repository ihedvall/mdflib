/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file mdffactory.h
 * \brief Factory class for the MDF library.
 */
#pragma once
#include <functional>
#include <cstdint>
#include <string>
#include <memory>

namespace mdf {
/** \brief MDF writer types. */
enum class MdfWriterType : int {
  Mdf3Basic = 0, ///< Basic MDF version 3 writer.
  Mdf4Basic = 1,  ///< Basic MDF version 4 writer.
  MdfBusLogger = 2, ///< Specialized bus logger writer.
  MdfConverter = 3, ///< MDF writer for MDF 4 conversion applications.
};

/** \brief MDF file type. */
enum class MdfFileType : int {
  Mdf3FileType = 0, ///< MDF version 3 file.
  Mdf4FileType = 1  ///< MDF version 4 file.
};

/** \brief Defines the log severity level. */
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

/** \brief MDF logging function definition. */
using MdfLogFunction2 = std::function<void(MdfLogSeverity severity,
  const std::string& function, const std::string& text)>;

class MdfWriter;
class MdfFile;

/** \brief MDF factory class. */
class MdfFactory {
 public:
  /** \brief Creates an MDF writer object. */
  static std::unique_ptr<MdfWriter> CreateMdfWriter(MdfWriterType type);
  /** \brief Create an MDF file object.*/
  static std::unique_ptr<MdfFile> CreateMdfFile(MdfFileType type);

  /** \brief Creates an MDF writer object. */
  static MdfWriter* CreateMdfWriterEx(MdfWriterType type);
  /** \brief Create an MDF file object.*/
  static MdfFile* CreateMdfFileEx(MdfFileType type);

  /** \brief Sets the log function. */
  static void SetLogFunction2(const MdfLogFunction2& func);
};

}  // namespace mdf