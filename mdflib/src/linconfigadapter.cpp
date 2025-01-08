/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "linconfigadapter.h"

#include "mdf/ichannelgroup.h"
#include "mdf/mdflogstream.h"

namespace mdf {
LinConfigAdapter::LinConfigAdapter(const MdfWriter& writer)
    : BusConfigAdapter(writer) {
  bus_type_ = MdfBusType::LIN;
  bus_name_ = "LIN";
}

void LinConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  // const bool mandatory = writer_.MandatoryMembersOnly();

  if (IChannelGroup* cg_frame = dg_block.CreateChannelGroup("LIN_Frame");
    cg_frame != nullptr) {
    cg_frame->PathSeparator('.');
    cg_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_frame);
    CreateTimeChannel(*cg_frame, "t");
    CreateFrameChannels(*cg_frame);
  }

  if (IChannelGroup* cg_checksum_error = dg_block.CreateChannelGroup("LIN_ChecksumError");
      cg_checksum_error != nullptr ) {
    cg_checksum_error->PathSeparator('.');
    cg_checksum_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_checksum_error);
    CreateTimeChannel(*cg_checksum_error, "t");
    CreateChecksumErrorChannels(*cg_checksum_error);
  }

  if (IChannelGroup* cg_receive_error = dg_block.CreateChannelGroup("LIN_ReceiveError");
      cg_receive_error != nullptr ) {
    cg_receive_error->PathSeparator('.');
    cg_receive_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_receive_error);
    CreateTimeChannel(*cg_receive_error, "t");
    CreateReceiveChannels(*cg_receive_error);
  }

  if (IChannelGroup* cg_sync_error = dg_block.CreateChannelGroup("LIN_SyncError");
      cg_sync_error != nullptr ) {
    cg_sync_error->PathSeparator('.');
    cg_sync_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_sync_error);
    CreateTimeChannel(*cg_sync_error, "t");
    CreateSyncChannels(*cg_sync_error);
  }
  if (IChannelGroup* cg_transmit_error =
          dg_block.CreateChannelGroup("LIN_TransmissionError");
      cg_transmit_error != nullptr ) {
    cg_transmit_error->PathSeparator('.');
    cg_transmit_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_transmit_error);
    CreateTimeChannel(*cg_transmit_error, "t");
    CreateTransmissionErrorChannels(*cg_transmit_error);
  }

  // if (mandatory) {
  //  return;
  // }

  if (IChannelGroup* cg_wakeUp = dg_block.CreateChannelGroup("LIN_WakeUp");
      cg_wakeUp != nullptr ) {
    cg_wakeUp->PathSeparator('.');
    cg_wakeUp->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_wakeUp);
    CreateTimeChannel(*cg_wakeUp, "t");
    CreateWakeUpChannels(*cg_wakeUp);
  }

  if (IChannelGroup* cg_spike = dg_block.CreateChannelGroup("LIN_Spike");
      cg_spike != nullptr ) {
    cg_spike->PathSeparator('.');
    cg_spike->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_spike);
    CreateTimeChannel(*cg_spike, "t");
    CreateSpikeChannels(*cg_spike);
  }

  if (IChannelGroup* cg_long_dom =
          dg_block.CreateChannelGroup("LIN_LongDom");
      cg_long_dom != nullptr ) {
    cg_long_dom->PathSeparator('.');
    cg_long_dom->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_long_dom);
    CreateTimeChannel(*cg_long_dom, "t");
    CreateLongDominantChannels(*cg_long_dom);
  }
}

void LinConfigAdapter::CreateFrameChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 11 : 36);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  if (IChannel* frame_bus =
          cn_frame->CreateChannelComposition(group_name + ".BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 0x3F);
    frame_bus->ByteOffset(8 + 0);
    frame_bus->BitOffset(0);
    frame_bus->BitCount(6);
  }

  if (IChannel* frame_id = cn_frame->CreateChannelComposition(group_name + ".ID");
      frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->Flags(CnFlag::BusEvent);
    frame_id->ByteOffset(8 + 1);
    frame_id->BitOffset(0);
    frame_id->BitCount(6);
  }

  if (IChannel* dir = CreateBitChannel(*cn_frame, group_name +".Dir", 8 + 1, 7);
      dir != nullptr) {
    if (IChannelConversion* cc_dir = dir->CreateChannelConversion();
        cc_dir != nullptr) {
      cc_dir->Type(ConversionType::ValueToText);
      cc_dir->Parameter(0, 0.0);
      cc_dir->Parameter(1, 1.0);
      cc_dir->Reference(0, "Rx");
      cc_dir->Reference(1, "Tx");
      cc_dir->Reference(2, "");  // Default text
    }
  }

  if (IChannel* nof_rec =
          cn_frame->CreateChannelComposition(group_name + ".ReceivedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 2);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(4);
  }

  if (IChannel* frame_length =
          cn_frame->CreateChannelComposition(group_name + ".DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8 + 2);
    frame_length->BitOffset(4);
    frame_length->BitCount(4);
  }

  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition(group_name + ".DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::MaxLength);
    frame_bytes->BitCount(8 * 8);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(11);
    frame_bytes->BitOffset(0);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* crc = cn_frame->CreateChannelComposition(group_name + ".Checksum");
      crc != nullptr) {
    crc->Type(ChannelType::FixedLength);
    crc->Sync(ChannelSyncType::None);
    crc->DataType(ChannelDataType::UnsignedIntegerLe);
    crc->Flags(CnFlag::BusEvent);
    crc->ByteOffset(19);
    crc->BitOffset(0);
    crc->BitCount(8);
  }

  if (IChannel* crc_model =
          cn_frame->CreateChannelComposition(group_name + ".ChecksumModel");
      crc_model != nullptr) {
    crc_model->Type(ChannelType::FixedLength);
    crc_model->Sync(ChannelSyncType::None);
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
    crc_model->Flags(CnFlag::BusEvent);
    crc_model->ByteOffset(8 + 0);
    crc_model->BitOffset(6);
    crc_model->BitCount(2);

    if (IChannelConversion* cc_type = crc_model->CreateChannelConversion();
        cc_type != nullptr) {
      cc_type->Type(ConversionType::ValueToText);
      cc_type->Parameter(0, -1.0);
      cc_type->Parameter(1, 0.0);
      cc_type->Parameter(2, 1.0);
      cc_type->Reference(0, "Unknown");
      cc_type->Reference(1, "Classic");
      cc_type->Reference(2, "Enhanced");
      cc_type->Reference(3, "");  // Default text
    }
  }

  if (IChannel* sof = cn_frame->CreateChannelComposition(group_name + ".SOF");
      sof != nullptr) {
    sof->Type(ChannelType::FixedLength);
    sof->Sync(ChannelSyncType::None);
    sof->DataType(ChannelDataType::UnsignedIntegerLe);
    sof->Flags(CnFlag::BusEvent);
    sof->ByteOffset(20);
    sof->BitOffset(0);
    sof->BitCount(8 * 8);
    sof->Unit("ns");
  }

  if (IChannel* baudrate =
          cn_frame->CreateChannelComposition(group_name + ".Baudrate");
      baudrate != nullptr) {
    baudrate->Type(ChannelType::FixedLength);
    baudrate->Sync(ChannelSyncType::None);
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Flags(CnFlag::BusEvent);
    baudrate->ByteOffset(28);
    baudrate->BitOffset(0);
    baudrate->BitCount(8 * 4);
    baudrate->Unit("bits/s");
  }

  if (IChannel* resp =
          cn_frame->CreateChannelComposition(group_name + ".ResponseBaudrate");
      resp != nullptr) {
    resp->Type(ChannelType::FixedLength);
    resp->Sync(ChannelSyncType::None);
    resp->DataType(ChannelDataType::FloatLe);
    resp->Flags(CnFlag::BusEvent);
    resp->ByteOffset(32);
    resp->BitOffset(0);
    resp->BitCount(8 * 4);
    resp->Unit("bits/s");
  }

  if (IChannel* break_length = cn_frame->CreateChannelComposition(
          group_name + ".BreakLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(36);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }

  if (IChannel* break_length =
          cn_frame->CreateChannelComposition(group_name + ".BreakDelimiterLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(40);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }
}

void LinConfigAdapter::CreateChecksumErrorChannels(mdf::IChannelGroup& group) {
  CreateFrameChannels(group);
}

void LinConfigAdapter::CreateReceiveChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_ReceiveError channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 2 : 35);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  if (IChannel* frame_bus =
          cn_frame->CreateChannelComposition(group_name + ".BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 0x3F);
    frame_bus->ByteOffset(8 + 0);
    frame_bus->BitOffset(0);
    frame_bus->BitCount(6);
  }

  if (IChannel* frame_id = cn_frame->CreateChannelComposition(group_name + ".ID");
      frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->Flags(CnFlag::BusEvent);
    frame_id->ByteOffset(8 + 1);
    frame_id->BitOffset(0);
    frame_id->BitCount(6);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* nof_spec =
          cn_frame->CreateChannelComposition(group_name + ".SpecifiedDataByteCount");
      nof_spec != nullptr) {
    nof_spec->Type(ChannelType::FixedLength);
    nof_spec->Sync(ChannelSyncType::None);
    nof_spec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_spec->Flags(CnFlag::BusEvent);
    nof_spec->ByteOffset(8 + 4);
    nof_spec->BitOffset(0);
    nof_spec->BitCount(4);
  }

  if (IChannel* nof_rec =
          cn_frame->CreateChannelComposition(group_name + ".ReceivedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 2);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(4);
  }

  if (IChannel* frame_length =
          cn_frame->CreateChannelComposition(group_name + ".DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8 + 2);
    frame_length->BitOffset(4);
    frame_length->BitCount(4);
  }

  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition(group_name + ".DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::MaxLength);
    frame_bytes->BitCount(8 * 8);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(13);
    frame_bytes->BitOffset(0);
  }

  if (IChannel* crc = cn_frame->CreateChannelComposition(group_name + ".Checksum");
      crc != nullptr) {
    crc->Type(ChannelType::FixedLength);
    crc->Sync(ChannelSyncType::None);
    crc->DataType(ChannelDataType::UnsignedIntegerLe);
    crc->Flags(CnFlag::BusEvent);
    crc->ByteOffset(8 + 3);
    crc->BitOffset(0);
    crc->BitCount(8);
  }

  if (IChannel* crc_model =
          cn_frame->CreateChannelComposition(group_name + ".ChecksumModel");
      crc_model != nullptr) {
    crc_model->Type(ChannelType::FixedLength);
    crc_model->Sync(ChannelSyncType::None);
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
    crc_model->Flags(CnFlag::BusEvent);
    crc_model->ByteOffset(8 + 0);
    crc_model->BitOffset(6);
    crc_model->BitCount(2);

    if (IChannelConversion* cc_type = crc_model->CreateChannelConversion();
        cc_type != nullptr) {
      cc_type->Type(ConversionType::ValueToText);
      cc_type->Parameter(0, -1.0);
      cc_type->Parameter(1, 0.0);
      cc_type->Parameter(2, 1.0);
      cc_type->Reference(0, "Unknown");
      cc_type->Reference(1, "Classic");
      cc_type->Reference(2, "Enhanced");
      cc_type->Reference(3, "");  // Default text
    }
  }

  if (IChannel* sof = cn_frame->CreateChannelComposition(group_name + ".SOF");
      sof != nullptr) {
    sof->Type(ChannelType::FixedLength);
    sof->Sync(ChannelSyncType::None);
    sof->DataType(ChannelDataType::UnsignedIntegerLe);
    sof->Flags(CnFlag::BusEvent);
    sof->ByteOffset(19);
    sof->BitOffset(0);
    sof->BitCount(8 * 8);
    sof->Unit("ns");
  }

  if (IChannel* baudrate =
          cn_frame->CreateChannelComposition(group_name + ".Baudrate");
      baudrate != nullptr) {
    baudrate->Type(ChannelType::FixedLength);
    baudrate->Sync(ChannelSyncType::None);
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Flags(CnFlag::BusEvent);
    baudrate->ByteOffset(27);
    baudrate->BitOffset(0);
    baudrate->BitCount(8 * 4);
    baudrate->Unit("bits/s");
  }

  if (IChannel* resp =
          cn_frame->CreateChannelComposition(group_name + ".ResponseBaudrate");
      resp != nullptr) {
    resp->Type(ChannelType::FixedLength);
    resp->Sync(ChannelSyncType::None);
    resp->DataType(ChannelDataType::FloatLe);
    resp->Flags(CnFlag::BusEvent);
    resp->ByteOffset(31);
    resp->BitOffset(0);
    resp->BitCount(8 * 4);
    resp->Unit("bits/s");
  }

  if (IChannel* break_length = cn_frame->CreateChannelComposition(
          group_name + ".BreakLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(35);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }

  if (IChannel* break_length =
          cn_frame->CreateChannelComposition(group_name + ".BreakDelimiterLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(39);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }
}

void LinConfigAdapter::CreateSyncChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_SyncError channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 5 : 21);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  if (IChannel* frame_bus =
          cn_frame->CreateChannelComposition(group_name + ".BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 0x3F);
    frame_bus->ByteOffset(8 + 0);
    frame_bus->BitOffset(0);
    frame_bus->BitCount(6);
  }

  if (IChannel* baudrate =
          cn_frame->CreateChannelComposition(group_name + ".Baudrate");
      baudrate != nullptr) {
    baudrate->Type(ChannelType::FixedLength);
    baudrate->Sync(ChannelSyncType::None);
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Flags(CnFlag::BusEvent);
    baudrate->ByteOffset(9);
    baudrate->BitOffset(0);
    baudrate->BitCount(8 * 4);
    baudrate->Unit("bits/s");
  }

  if (mandatory) {
    return;
  }

  if (IChannel* sof = cn_frame->CreateChannelComposition(group_name + ".SOF");
      sof != nullptr) {
    sof->Type(ChannelType::FixedLength);
    sof->Sync(ChannelSyncType::None);
    sof->DataType(ChannelDataType::UnsignedIntegerLe);
    sof->Flags(CnFlag::BusEvent);
    sof->ByteOffset(13);
    sof->BitOffset(0);
    sof->BitCount(8 * 8);
    sof->Unit("ns");
  }

  if (IChannel* break_length = cn_frame->CreateChannelComposition(
          group_name + ".BreakLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(21);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }

  if (IChannel* del_break_length =
          cn_frame->CreateChannelComposition(group_name + ".BreakDelimiterLength");
      del_break_length != nullptr) {
    del_break_length->Type(ChannelType::FixedLength);
    del_break_length->Sync(ChannelSyncType::None);
    del_break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    del_break_length->Flags(CnFlag::BusEvent);
    del_break_length->ByteOffset(25);
    del_break_length->BitOffset(0);
    del_break_length->BitCount(8 * 4);
    del_break_length->Unit("ns");
  }
}

void LinConfigAdapter::CreateWakeUpChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_SyncError channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 1 : 13);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  if (IChannel* frame_bus =
          cn_frame->CreateChannelComposition(group_name + ".BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 0x3F);
    frame_bus->ByteOffset(8 + 0);
    frame_bus->BitOffset(0);
    frame_bus->BitCount(6);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* baudrate =
          cn_frame->CreateChannelComposition(group_name + ".Baudrate");
      baudrate != nullptr) {
    baudrate->Type(ChannelType::FixedLength);
    baudrate->Sync(ChannelSyncType::None);
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Flags(CnFlag::BusEvent);
    baudrate->ByteOffset(9);
    baudrate->BitOffset(0);
    baudrate->BitCount(8 * 4);
    baudrate->Unit("bits/s");
  }

  if (IChannel* sof = cn_frame->CreateChannelComposition(group_name + ".SOF");
      sof != nullptr) {
    sof->Type(ChannelType::FixedLength);
    sof->Sync(ChannelSyncType::None);
    sof->DataType(ChannelDataType::UnsignedIntegerLe);
    sof->Flags(CnFlag::BusEvent);
    sof->ByteOffset(13);
    sof->BitOffset(0);
    sof->BitCount(8 * 8);
    sof->Unit("ns");
  }
}

void LinConfigAdapter::CreateTransmissionErrorChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name: " << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 2 : 23);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  if (IChannel* frame_bus =
          cn_frame->CreateChannelComposition(group_name + ".BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 0x3F);
    frame_bus->ByteOffset(8 + 0);
    frame_bus->BitOffset(0);
    frame_bus->BitCount(6);
  }

  if (IChannel* frame_id = cn_frame->CreateChannelComposition(group_name + ".ID");
      frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->Flags(CnFlag::BusEvent);
    frame_id->ByteOffset(8 + 1);
    frame_id->BitOffset(0);
    frame_id->BitCount(6);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* nof_rec =
      cn_frame->CreateChannelComposition(group_name + ".SpecifiedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 2);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(4);
  }

  if (IChannel* crc_model =
          cn_frame->CreateChannelComposition(group_name + ".ChecksumModel");
      crc_model != nullptr) {
    crc_model->Type(ChannelType::FixedLength);
    crc_model->Sync(ChannelSyncType::None);
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
    crc_model->Flags(CnFlag::BusEvent);
    crc_model->ByteOffset(8 + 0);
    crc_model->BitOffset(6);
    crc_model->BitCount(2);

    if (IChannelConversion* cc_type = crc_model->CreateChannelConversion();
        cc_type != nullptr) {
      cc_type->Type(ConversionType::ValueToText);
      cc_type->Parameter(0, -1.0);
      cc_type->Parameter(1, 0.0);
      cc_type->Parameter(2, 1.0);
      cc_type->Reference(0, "Unknown");
      cc_type->Reference(1, "Classic");
      cc_type->Reference(2, "Enhanced");
      cc_type->Reference(3, "");  // Default text
    }
  }

  if (IChannel* baudrate =
          cn_frame->CreateChannelComposition(group_name + ".Baudrate");
      baudrate != nullptr) {
    baudrate->Type(ChannelType::FixedLength);
    baudrate->Sync(ChannelSyncType::None);
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Flags(CnFlag::BusEvent);
    baudrate->ByteOffset(11);
    baudrate->BitOffset(0);
    baudrate->BitCount(8 * 4);
    baudrate->Unit("bits/s");
  }

  if (IChannel* sof = cn_frame->CreateChannelComposition(group_name + ".SOF");
      sof != nullptr) {
    sof->Type(ChannelType::FixedLength);
    sof->Sync(ChannelSyncType::None);
    sof->DataType(ChannelDataType::UnsignedIntegerLe);
    sof->Flags(CnFlag::BusEvent);
    sof->ByteOffset(15);
    sof->BitOffset(0);
    sof->BitCount(8 * 8);
    sof->Unit("ns");
  }

  if (IChannel* break_length = cn_frame->CreateChannelComposition(
          group_name + ".BreakLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(23);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }

  if (IChannel* break_length =
          cn_frame->CreateChannelComposition(group_name + ".BreakDelimiterLength");
      break_length != nullptr) {
    break_length->Type(ChannelType::FixedLength);
    break_length->Sync(ChannelSyncType::None);
    break_length->DataType(ChannelDataType::UnsignedIntegerLe);
    break_length->Flags(CnFlag::BusEvent);
    break_length->ByteOffset(27);
    break_length->BitOffset(0);
    break_length->BitCount(8 * 4);
    break_length->Unit("ns");
  }
}

void LinConfigAdapter::CreateSpikeChannels(IChannelGroup& group) {
  CreateWakeUpChannels(group);
}

void LinConfigAdapter::CreateLongDominantChannels(mdf::IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name: " << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 2 : 18);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  if (IChannel* frame_bus =
          cn_frame->CreateChannelComposition(group_name + ".BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 0x3F);
    frame_bus->ByteOffset(8 + 0);
    frame_bus->BitOffset(0);
    frame_bus->BitCount(6);
  }
  if (IChannel* type = cn_frame->CreateChannelComposition(group_name + ".Type");
      type != nullptr) {
    type->Type(ChannelType::FixedLength);
    type->Sync(ChannelSyncType::None);
    type->DataType(ChannelDataType::UnsignedIntegerLe);
    type->Flags(CnFlag::BusEvent);
    type->ByteOffset(8 + 1);
    type->BitOffset(4);
    type->BitCount(2);

    if (IChannelConversion* cc_type = type->CreateChannelConversion();
        cc_type != nullptr) {
      cc_type->Type(ConversionType::ValueToText);
      cc_type->Parameter(0, 0.0);
      cc_type->Parameter(1, 1.0);
      cc_type->Parameter(2, 2.0);
      cc_type->Reference(0, "First Detection");
      cc_type->Reference(1, "Cyclic Report");
      cc_type->Reference(2, "End of Detection");
      cc_type->Reference(3, "");  // Default text
    }
  }

  if (mandatory) {
    return;
  }

  if (IChannel* baudrate =
          cn_frame->CreateChannelComposition(group_name + ".Baudrate");
      baudrate != nullptr) {
    baudrate->Type(ChannelType::FixedLength);
    baudrate->Sync(ChannelSyncType::None);
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Flags(CnFlag::BusEvent);
    baudrate->ByteOffset(10);
    baudrate->BitOffset(0);
    baudrate->BitCount(8 * 4);
    baudrate->Unit("bits/s");
  }

  if (IChannel* sof = cn_frame->CreateChannelComposition(group_name + ".SOF");
      sof != nullptr) {
    sof->Type(ChannelType::FixedLength);
    sof->Sync(ChannelSyncType::None);
    sof->DataType(ChannelDataType::UnsignedIntegerLe);
    sof->Flags(CnFlag::BusEvent);
    sof->ByteOffset(14);
    sof->BitOffset(0);
    sof->BitCount(8 * 8);
    sof->Unit("ns");
  }

  if (IChannel* length = cn_frame->CreateChannelComposition(
          group_name + ".Length");
      length != nullptr) {
    length->Type(ChannelType::FixedLength);
    length->Sync(ChannelSyncType::None);
    length->DataType(ChannelDataType::UnsignedIntegerLe);
    length->Flags(CnFlag::BusEvent);
    length->ByteOffset(22);
    length->BitOffset(0);
    length->BitCount(8 * 4);
    length->Unit("ns");
  }


}

}  // namespace mdf