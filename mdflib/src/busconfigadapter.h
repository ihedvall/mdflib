/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdfwriter.h"
#include "mdf/idatagroup.h"


namespace mdf {

class IChannelGroup;
class IChannel;
class ISourceInformation;

class BusConfigAdapter {
 public:
  BusConfigAdapter() = delete;
  explicit BusConfigAdapter(const MdfWriter& writer);

  void BusName(std::string bus_name) { bus_name_ = std::move(bus_name); }

  virtual void CreateConfig(IDataGroup& dg_block) = 0;

 protected:
  uint16_t bus_type_ = 0;
  std::string bus_name_;
  const MdfWriter& writer_;

  static IChannel* CreateTimeChannel(IChannelGroup& group,
                              const std::string_view& name);

  ISourceInformation* CreateSourceInformation(IChannelGroup& group);
  static IChannel* CreateBitChannel(IChannel& parent,
                                    const std::string_view& name,
                                    uint32_t byte_offset,
                                    uint16_t bit_offset);
};

}  // namespace mdf


