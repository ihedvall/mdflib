/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "busconfigadapter.h"

#include "mdf/ichannelgroup.h"
#include "mdf/mdfwriter.h"

namespace mdf {

BusConfigAdapter::BusConfigAdapter(const MdfWriter& writer)
: writer_(writer) {

}

IChannel* BusConfigAdapter::CreateTimeChannel(IChannelGroup& group,
                            const std::string_view& name) {
  auto cn_list = group.Channels();

  // First check if a time channel exist. If so return it unchanged.
  const auto itr = std::find_if(cn_list.begin(), cn_list.end(),
             [] (const IChannel* channel) -> bool {
              return channel != nullptr && channel->Type() == mdf::ChannelType::Master;
             });
  if (itr != cn_list.end()) {
    return *itr;
  }

  IChannel* time = group.CreateChannel(name.data());
  if (time != nullptr) {
    time->Name(name.data());
    time->Description(group.Name());
    time->Type(ChannelType::Master);
    time->Sync(ChannelSyncType::Time);
    time->DataType(ChannelDataType::FloatLe);
    time->DataBytes(8);
    time->Unit("s");
  }
  return time;
}

ISourceInformation* BusConfigAdapter::CreateSourceInformation(IChannelGroup& group) {
  ISourceInformation* info = group.CreateSourceInformation();
  if (info != nullptr) {
    info->Name(bus_name_);
    info->Path(group.Name());
    info->Type(SourceType::Bus);

    switch (bus_type_) {
      case MdfBusType::CAN:
        info->Bus(BusType::Can);
        break;

      case MdfBusType::LIN:
        info->Bus(BusType::Lin);
        break;

      case MdfBusType::MOST:
        info->Bus(BusType::Most);
        break;

      case MdfBusType::FlexRay:
        info->Bus(BusType::FlexRay);
        break;

      case MdfBusType::Ethernet:
        info->Bus(BusType::Ethernet);
        break;

      default:
        info->Bus(BusType::Other);
        break;
    }
  }
  return info;
}

IChannel* BusConfigAdapter::CreateBitChannel(IChannel& parent,
                                const std::string_view& name,
                                uint32_t byte_offset,
                                uint16_t bit_offset) {
  auto* frame_bit = parent.CreateChannelComposition(name);
  if (frame_bit != nullptr) {
    frame_bit->Type(ChannelType::FixedLength);
    frame_bit->Sync(ChannelSyncType::None);
    frame_bit->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bit->Flags(CnFlag::BusEvent);
    frame_bit->ByteOffset(byte_offset);
    frame_bit->BitOffset(bit_offset);
    frame_bit->BitCount(1);
  }
  return frame_bit;
}

}  // namespace mdf