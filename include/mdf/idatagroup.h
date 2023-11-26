/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file idatagroup.h
 * \brief Interface to a data group (DG) block.
 *
 */
#pragma once
#include <string>
#include <vector>

#include "ichannelgroup.h"
#include "isampleobserver.h"
#include "mdf/iblock.h"

namespace mdf {

class IMetaData;

/** \brief Interface to a data group (DG) block.
 *
 * The data group block is the entry point for a measurement. The header (HD)
 * block is the entry point for a test. A test consist of one or more
 * measurements (DG).
 *
 * Each measurement is defined by one or more channel groups (CG). The channel
 * group in turn, consist of one or more channels. The group have an unique
 * record ID.
 *
 * A record is a byte array with fixed length and hold one sample for each
 * channel in the group. The DG block also points to the data blocks (DT) which
 * is an array of sample records.
 *
 * In MDF version 3, data was always stored as raw bytes in a DT block. In
 * version 4 the DT block may be split, reorganized and compressed.
 *
 */
class IDataGroup : public IBlock {
 public:

  /** \brief Sets the descriptive text for the measurement. */
  virtual void Description(const std::string& desc);
  /** \brief Return the descriptive text. */
  [[nodiscard]] virtual std::string Description() const;

  /** \brief Sets size of the record ID (bytes). Note that record
   * ID and its size, is automatically set when writing MDF files. */
  virtual void RecordIdSize(uint8_t id_size);
  /** \brief Returns the record ID size in bytes. */
  [[nodiscard]] virtual uint8_t RecordIdSize() const;

  /** \brief Returns a list of channel groups. */
  [[nodiscard]] virtual std::vector<IChannelGroup*> ChannelGroups() const = 0;

  /** \brief Create a new empty channel group. */
  [[nodiscard]] virtual IChannelGroup* CreateChannelGroup() = 0;

  /** \brief Create a new channel group or return the existing group. */
  [[nodiscard]] IChannelGroup* CreateChannelGroup(
      const std::string_view& name);

  /** \brief Returns the existing group by its name. */
  [[nodiscard]] IChannelGroup* GetChannelGroup(const std::string_view& name) const;

  /** \brief Return a channel group by its record id. */
  [[nodiscard]] IChannelGroup* GetChannelGroup(uint64_t record_id) const;

  /** \brief Create or return the existing meta-data (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData();
  /** \brief Returns the existing meta-data (MD) block if it exist. */
  [[nodiscard]] virtual IMetaData* MetaData() const;

  /** \brief Internal function that attach a sample observer to the
   *  measurement block.  */
  void AttachSampleObserver(ISampleObserver* observer) const;
  /** \brief Detach an observer from  the measurement. */
  void DetachSampleObserver(const ISampleObserver* observer) const;
  /** \brief Detaches all observers from the measurement. */
  void DetachAllSampleObservers() const;
  /** \brief Notifies the observer that a new sample record have been read.*/
  void NotifySampleObservers(size_t sample, uint64_t record_id,
                             const std::vector<uint8_t>& record) const;

  /** \brief Clear all temporary sample and data buffers. */
  void ResetSample() const;
  /** \brief Set the DG blocks data as read. */
  void SetAsRead(bool mark_as_read = true) const {
    mark_as_read_ = mark_as_read;
  }

  /** \brief Returns true if no samples has been stored yet. */
  [[nodiscard]] bool IsEmpty() const;

  /** \brief Return true if the DG blocks data has been read not the DG block
   * itself.  */
  [[nodiscard]] bool IsRead() const { return mark_as_read_; }

  /** \brief Support function that return the first CG block that contains
   * a specific CN block.
   */
  [[nodiscard]] virtual const IChannelGroup *FindParentChannelGroup(
      const IChannel &channel) const = 0;

 protected:
  mutable std::vector<ISampleObserver*> observer_list; ///< List of observers.
  ~IDataGroup() override = default; ///< Default destructor

 private:
  mutable bool mark_as_read_ = false; ///< True if the data block has been read.


};

}  // namespace mdf
