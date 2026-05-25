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

#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/ichannelobserver.h"
#include "mdf/idatawriter.h"

namespace mdf {


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
class IChannelObserver;

/**
 * \brief Factory class for creating MDF-related objects.
 *
 * The MdfFactory class provides static methods to create readers, writers,
 * file objects, and observers for working with MDF data. Additionally, it
 * allows setting custom log functions for debugging or monitoring purposes.
 */
class MdfFactory {
 public:

  /**
   * \brief Creates an MDF reader object.
   *
   * The function creates a smart pointer to an MDF reader object.
   *
   * \param filename Full path to the MDF file.
   */
  static std::unique_ptr<MdfReader> CreateMdfReader(std::string filename);

  /**
   * \brief Creates an MDF writer object.
   *
   * The function creates a smart pointer to an MDF writer object.
   *
   * \param type Type of MDF writer to create.
   */
  static std::unique_ptr<MdfWriter> CreateMdfWriter(MdfWriterType type);

  /**
   * \brief Create an MDF file object.
   *
   * The function creates a smart pointer to an MDF file object.
   *
   * \param type Type of MDF file to create.
   */
  static std::unique_ptr<MdfFile> CreateMdfFile(MdfFileType type);

  /**
   * \brief Creates an MDF writer object.
   *
   * The function creates a raw pointer to an MDF writer object.
   * This function should be used with caution as it returns a raw pointer.
   *
   * \param type Type of MDF writer to create.
   */
  static MdfWriter* CreateMdfWriterEx(MdfWriterType type);

  /**
   * \brief Create an MDF file object.
   *
   * The function creates a raw pointer to an MDF file object.
   * This function should be used with caution as it returns a raw pointer.
   *
   * \param type Type of MDF file to create.
   */
  static MdfFile* CreateMdfFileEx(MdfFileType type);

  /**
   * \brief Sets the log function.
   *
   * The function sets the log function for the MDF library.
   *
   * \param func The log function to set.
   */
  static void SetLogFunction2(const MdfLogFunction2& func);

  /**
   * \brief Creates a channel observer object.
   *
   * The function creates a smart pointer to a channel observer object.
   *
   * \param data_group The data group to observe.
   * \param channel_group The channel group to observe.
   * \param channel The channel to observe.
   */
  static std::unique_ptr<IChannelObserver> CreateChannelObserver(
    const IDataGroup& data_group, const IChannelGroup& channel_group,
    const IChannel& channel);

  static std::unique_ptr<IDataWriter>
  CreateDataWriter(IChannelGroup& channel_group,
    MdfFileType type = MdfFileType::Mdf4FileType);

};

}  // namespace mdf