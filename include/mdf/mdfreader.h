/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdio>
#include <string>
#include <memory>
#include "mdf/mdffile.h"

namespace mdf {

using ChannelObserverPtr = std::unique_ptr<IChannelObserver>;
using ChannelObserverList = std::vector<ChannelObserverPtr>;

/// \brief Returns true if the file is an MDF file.
[[nodiscard]] bool IsMdfFile(const std::string &filename);


/// \brief Creates and attaches a channel sample observer.
[[nodiscard]] ChannelObserverPtr CreateChannelObserver(const IDataGroup& data_group,
                                                       const IChannelGroup& group,
                                                       const IChannel& channel);
/**
 * Function that creates a channel observer by searching the channel name. In case that the
 * channel exist in many channel groups, it selects the group with the largest number of samples.
 * @param dg_group The data group with the channel.
 * @param channel_name The channel name
 * @return A smart pointer to a channel observer.
 */
[[nodiscard]] ChannelObserverPtr CreateChannelObserver(const IDataGroup& dg_group, const std::string& channel_name);

void CreateChannelObserverForChannelGroup(const IDataGroup& data_group,
                     const IChannelGroup& group, ChannelObserverList& dest);

/** \class MdfReader mdfreader.h "mdf/mdfreader.h"
 * \brief Reader interface to an MDF file.
 *
 * This is the main interface when reading MDF3 and MDF4 files.
 */
 class MdfReader {
 public:
  explicit MdfReader(const std::string &filename); ///< Constructor that opens the file and read ID and HD block.
  virtual~MdfReader(); ///< Destructor that close any open file and destructs.

  MdfReader() = delete;
  MdfReader(const MdfReader &) = delete;
  MdfReader(MdfReader &&) = delete;
  MdfReader &operator=(const MdfReader &) = delete;
  MdfReader &operator=(MdfReader &&) = delete;

  /**
   * Unique index for this reader. This index is typically used when fetching files from a
   * database. The index itself is not used by the reader.
   * @return An unique index.
   */
  [[nodiscard]] int64_t Index() const {
    return index_;
  }

  /**
   * Sets a unique index to this reader or actually its file. This index is typically retrieved from a
   * database and makes finding files much easier than comparing paths.
   * @param index Unique index.
   */
  void Index(int64_t index) {
    index_ = index;
  }

  /// Checks if the file was read without errors.
  /// \return True if the file was read OK.
  [[nodiscard]] bool IsOk() const {
    return static_cast<bool>(instance_);
  }

  /// Returns a pointer to the MDF file. This file holds references to the MDF blocks.
  /// \return Pointer to the MDF file object. Note it may return a null pointer.
  [[nodiscard]] const MdfFile *GetFile() const {
    return instance_.get();
  }

  [[nodiscard]] const IDataGroup* GetDataGroup(size_t order) const;

  [[nodiscard]] std::string ShortName() const; ///< Returns the file name without paths.

  bool Open(); ///< Opens the file stream for reading.
  void Close();///< Closes the file stream.

  bool ReadHeader(); ///< Reads the ID and the HD block.
  bool ReadMeasurementInfo(); ///< Reads everything but not CG and raw data.
  bool ReadEverythingButData(); ///< Reads all blocks but not raw data.

  /// \brief Saves the attached data into a destination file.
  bool ReadAttachmentData(const IAttachment& attachment, const std::string& dest_file);

  bool ReadData(const IDataGroup& data_group); ///< Reads the sample data. See sample observer.

 private:
  std::FILE *file_ = nullptr; ///< Pointer to the file stream.
  std::string filename_; ///< The file name with full path.
  std::unique_ptr<MdfFile> instance_; ///< Pointer to the MDF file object.
  int64_t index_ = 0; ///< Unique (database) file index that can be used to identify a file instead of its path.

};
}
