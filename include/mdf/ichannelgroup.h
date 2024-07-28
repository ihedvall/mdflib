/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelgroup.h
 * \brief Defines an interface against a channel group (CG) block.
 */
#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include "ichannel.h"
#include "samplerecord.h"

#include "mdf/iblock.h"
#include "mdf/imetadata.h"

namespace mdf {
/** \brief Channel group flags. */
namespace CgFlag {
/** \brief Flag is used to indicate the this block is a variable length CG
 * block.
 *
 * If a CG block is marked as a variable length data (VLSD) block. This
 * technique is used when writing byte arrays with variable length. Bus
 * recorders uses this technique instead of using SD blocks.
 *
 * The problem with
 * SD blocks is that they need to be temporary stored in primary memory. For
 * long time recording this is not acceptable due to the risk of loss of power
 * during th measurement.
 *
 * The draw-back is that the VLSD CG block requires some extra internal
 * functionality both for reading and writing.
 */
constexpr uint16_t VlsdChannel = 0x0001;

/** \brief Bus event flag. */
constexpr uint16_t BusEvent = 0x0002;

/** \brief Plain bus event flag. */
constexpr uint16_t PlainBusEvent = 0x0004;

/** \brief Remote master flag.*/
constexpr uint16_t RemoteMaster = 0x0008;

/** \brief Event signal group. The group store events not values. */
constexpr uint16_t EventSignal = 0x00010;
}  // namespace CgFlag

class ISourceInformation;

/** \brief Interface against a channel group (CG) block.
 *
 * A channel group defines a group of signals that are sampled simultaneously.
 * So the number of samples are the same for the channels. Each data sample is
 * stored in so-called record buffer. The record buffer normally have fixed
 * length.
 *
 * The above is somewhat not correct because if the CG block doesn't have any
 * signals, it stores some other data which typical is used when logging bus
 * messages.
 */
class IChannelGroup : public IBlock {
 public:

  virtual void RecordId(uint64_t record_id) = 0; ///< Sets the record identity.
  [[nodiscard]] virtual uint64_t RecordId() const = 0; ///< Record identity.

  virtual void Name(const std::string& name) = 0; ///< Sets the name.
  [[nodiscard]] virtual std::string Name() const = 0; ///< CG name.

  virtual void Description(const std::string& description)
      = 0; ///< Sets a descriptive text.
  [[nodiscard]] virtual std::string Description() const = 0; ///< Description.

  [[nodiscard]] virtual uint64_t NofSamples() const
      = 0; ///< Sets number of samples.
  virtual void NofSamples(uint64_t nof_samples) = 0; ///< Number of samples

  [[nodiscard]] virtual uint16_t Flags() const; ///< Sets CgFlag.
  virtual void Flags(uint16_t flags); ///< Returns CgFlag.

  [[nodiscard]] virtual char16_t PathSeparator(); ///< Sets the path separator.
  virtual void PathSeparator(char16_t path_separator); ///< Path separator.

  /** \brief Returns a list of channels. */
  [[nodiscard]] virtual std::vector<IChannel*> Channels() const = 0;

  /** \brief Creates a new channel. */
  [[nodiscard]] virtual IChannel* CreateChannel() = 0;

  /** \brief Creates a new channel or returns an existing channel. */
  [[nodiscard]] virtual IChannel* CreateChannel(const std::string_view& name);

  /** \brief Returns an existing channels part of name.
   *
   * Note that the function search for a name that includes the search name.
   * Example if the search name is '.DataLength', the signal with the name
   * 'CAN_DataFrame.DataLength' will be returned
   * the name instead of the full name */
  [[nodiscard]] virtual IChannel* GetChannel(const std::string_view& name) const;


  /** \brief Returns an external reference channel. */
  [[nodiscard]] virtual const IChannel* GetXChannel(
      const IChannel& reference) const = 0;

  /** \brief Create a source information (SI) block. */
  [[nodiscard]] virtual ISourceInformation* CreateSourceInformation();

  [[nodiscard]] virtual ISourceInformation* SourceInformation()
      const; ///< Returns the source information (SI) block if it exist. */

  /** \brief Support function that creates a sample record. */
  [[nodiscard]] SampleRecord GetSampleRecord() const;

  /** \brief Resets the internal sample counter. Internal use only. */
  void ResetSampleCounter() const { sample_ = 0;}

  virtual void ClearData(); ///< Resets all temporary stored samples.
  void IncrementSample() const; ///< Add a sample

  [[nodiscard]] size_t Sample() const; ///< Returns number of samples.

  /** \brief Creates a meta-data (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData();

  /** \brief Returns the meta-data (MD) block if it exist. */
  [[nodiscard]] virtual IMetaData* MetaData() const;

  /** \brief Returns a pointer to data group (DG) block. */
  [[nodiscard]] virtual const IDataGroup* DataGroup() const = 0;

 protected:
  mutable std::vector<uint8_t>
      sample_buffer_;  ///< Temporary record when saving samples.
 private:
  mutable size_t sample_ = 0;  ///< Support for the OnSample observers
};

}  // namespace mdf
