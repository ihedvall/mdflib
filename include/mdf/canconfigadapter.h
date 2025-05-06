/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string_view>
#include <string>
#include "mdf/iconfigadapter.h"

namespace mdf {


class CanConfigAdapter : public IConfigAdapter {
 public:
  CanConfigAdapter() = delete;
  explicit CanConfigAdapter(const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;
 protected:

  /** \brief Create the composition channels for a data frame
  *
  * The composition layout is as above. Note that the
  * \verbatim
  * Byte Remarks
  * 0:   LSB Message ID 32-bit unsigned
  * 1:
  * 2:
  * 3:   MSB Bit 31 is the IDE (extended bit)
  * 4:   BusChannel (High 4 bit), DLC (Low 4 bits)
  * 5:   Flags (8 bits)
  * 6:   64-bit Index into Signal Data or VLSD block. For MLSD
  * 7:   this stores the data bytes
  * ..
  * N:
  * \endverbatim
  * @param group The The CAN Data Frame channel group object.
   */
  virtual void CreateCanDataFrameChannel(IChannelGroup& group) const;

  /** \brief Create the composition channels for a remote frame
  *
  * The composition layout is as above. Note that the
  * \verbatim
  * Byte Remarks
  * 0:   LSB Message ID 32-bit unsigned
  * 1:
  * 2:
  * 3:   MSB Bit 31 is the IDE (extended bit)
  * 4:   BusChannel (High 4 bit), DLC (Low 4 bits)
  * 5:   Flags (8 bits)
  * \endverbatim
  * @param group The The CAN Remote Frame channel group object.
   */
  virtual void CreateCanRemoteFrameChannel(IChannelGroup& group) const;

  /** \brief Create the composition channels for an error frame
  *
  * The composition layout is as above. Note that the
  * \verbatim
  * Byte Remarks
  * 0:   LSB Message ID 32-bit unsigned
  * 1:
  * 2:
  * 3:   MSB Bit 31 is the IDE (extended bit)
  * 4:   BusChannel (High 4 bit), DLC (Low 4 bits)
  * 5:   Flags (8 bits)
  * 6:   Bit position
  * 7:   Error Type
  * 8:   64-bit Index into Signal Data or VLSD block. For MLSD
  * 9:   this stores the data bytes
  * ..
  * N:
  * \endverbatim
  * @param group The The CAN Error Frame channel group object.
   */
  virtual void CreateCanErrorFrameChannel(IChannelGroup& group) const;

  /** \brief Create the composition channels for an error frame
  *
  * The composition layout is as above. Note that the
  * \verbatim
  * Byte Remarks
  * 0:   BusChannel (High 4 bit), Flags (Low 4 bits)
  * \endverbatim
  * @param group The The CAN Overload Frame channel group object.
   */
  virtual void CreateCanOverloadFrameChannel(IChannelGroup& group);



};

}  // namespace mdf


