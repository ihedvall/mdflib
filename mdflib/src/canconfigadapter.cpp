/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/canconfigadapter.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdflogstream.h"

namespace mdf {

CanConfigAdapter::CanConfigAdapter(const MdfWriter& writer)
    : IConfigAdapter(writer) {
  BusType(MdfBusType::CAN);
  BusName("CAN");
}

void CanConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  const bool mandatory_only = MandatoryMembersOnly();
  // The cn_data_byte points to the channel that defines the raw data bytes.
  // This channel may be updated later to pint to the CG VLSD group.
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_data_frame = dg_block.CreateChannelGroup(
          MakeGroupName("DataFrame"));
      cg_data_frame != nullptr) {
    cg_data_frame->PathSeparator('.');
    cg_data_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_data_frame,"t");
    CreateCanDataFrameChannel(*cg_data_frame);
    cn_data_byte = cg_data_frame->GetChannel("CAN_DataFrame.DataBytes");
    CreateSourceInformation(*cg_data_frame);
  }

  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (auto* cg_samples_frame = dg_block.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }

  if (auto* cg_remote_frame = dg_block.CreateChannelGroup(
          MakeGroupName("RemoteFrame"));
      cg_remote_frame != nullptr) {
    // The remote frame doesn't store any data bytes.
    cg_remote_frame->PathSeparator('.');
    cg_remote_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_remote_frame,"t");
    CreateCanRemoteFrameChannel(*cg_remote_frame);
    CreateSourceInformation(*cg_remote_frame);
  }

  // Similar to the data frame, the error frame may store data bytes.
  const IChannel* cn_error_byte = nullptr;
  if (auto* cg_error_frame = dg_block.CreateChannelGroup(
          MakeGroupName("ErrorFrame"));
      cg_error_frame != nullptr) {
    cg_error_frame->PathSeparator('.');
    cg_error_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_error_frame,"t");
    CreateCanErrorFrameChannel(*cg_error_frame);
    cn_error_byte = cg_error_frame->GetChannel("CAN_ErrorFrame.DataBytes");
    CreateSourceInformation(*cg_error_frame);
  }

  if (StorageType() == MdfStorageType::VlsdStorage && cn_error_byte != nullptr) {
    // Need to add a special CG group for the error samples
    if (auto* cg_errors_frame = dg_block.CreateChannelGroup("");
        cg_errors_frame != nullptr) {
      cg_errors_frame->Flags(CgFlag::VlsdChannel);
      cn_error_byte->VlsdRecordId(cg_errors_frame->RecordId());
    }
  }

  if (mandatory_only) {
    // Do not include the overload frame as it rarely is used.
    return;
  }

  if (auto* cg_overload_frame = dg_block.CreateChannelGroup(
          MakeGroupName("OverloadFrame"));
      cg_overload_frame != nullptr) {
    cg_overload_frame->PathSeparator('.');
    cg_overload_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_overload_frame,"t");
    CreateCanOverloadFrameChannel(*cg_overload_frame);
    CreateSourceInformation(*cg_overload_frame);
  }
}

void CanConfigAdapter::CreateCanDataFrameChannel(IChannelGroup& group) const {
  auto* cn_data_frame = group.CreateChannel("CAN_DataFrame");
  if (cn_data_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_DataFrame channel.";
    return;
  }

  cn_data_frame->Type(ChannelType::FixedLength);
  cn_data_frame->Sync(ChannelSyncType::None);
  switch (StorageType()) {
    // In reality,  MLSD storage should have max length equal to 8.
    case MdfStorageType::MlsdStorage:
      cn_data_frame->DataBytes(6 + writer_.MaxLength());
      break;

    case MdfStorageType::FixedLengthStorage:
    case MdfStorageType::VlsdStorage:
    default:
      cn_data_frame->DataBytes(6 + 8); // Index into SD or VLSD
      break;
  }
  cn_data_frame->Flags(CnFlag::BusEvent);
  cn_data_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_data_frame);
  }

  if (auto* frame_bus = cn_data_frame->CreateChannelComposition("CAN_DataFrame.BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0,15);
    frame_bus->ByteOffset(8+4);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
  }

  if (auto* frame_id = cn_data_frame->CreateChannelComposition("CAN_DataFrame.ID");
      frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->Flags(CnFlag::BusEvent);
    frame_id->ByteOffset(8);
    frame_id->BitOffset(0);
    frame_id->BitCount(29);
  }
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.IDE", 8 + 3, 7);

  if (auto* frame_dlc = cn_data_frame->CreateChannelComposition(
      "CAN_DataFrame.DLC");
      frame_dlc != nullptr) {
    frame_dlc->Type(ChannelType::FixedLength);
    frame_dlc->Sync(ChannelSyncType::None);
    frame_dlc->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_dlc->Flags(CnFlag::BusEvent);
    frame_dlc->ByteOffset(8+4);
    frame_dlc->BitOffset(0);
    frame_dlc->BitCount(4);
  }

  if (auto* frame_length = cn_data_frame->CreateChannelComposition(
      "CAN_DataFrame.DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8+4);
    frame_length->BitOffset(0);
    frame_length->BitCount(4);

    if (auto* cc_length = frame_length->CreateChannelConversion();
        cc_length != nullptr) {
      cc_length->Type(ConversionType::ValueToValue);
      uint16_t index = 0;
      for (uint8_t key = 0; key < 16; ++key) {
        cc_length->Parameter(index++,static_cast<double>(key));
        cc_length->Parameter(index++,
                             static_cast<double>(CanMessage::DlcToLength(key)));
      }
    }
  }

  if (auto* frame_bytes = cn_data_frame->CreateChannelComposition(
      "CAN_DataFrame.DataBytes");
      frame_bytes != nullptr) {

    switch(StorageType()) {
      case MdfStorageType::MlsdStorage:
        frame_bytes->Type(ChannelType::MaxLength);
        frame_bytes->BitCount(8 * writer_.MaxLength());
        break;

      case MdfStorageType::VlsdStorage:
      default:
        frame_bytes->Type(ChannelType::VariableLength);
        frame_bytes->BitCount(8 * 8); // Index to SD or VLSD CG block
        break;
    }
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(8+6);
    frame_bytes->BitOffset(0);
  }

  if (auto* dir = CreateBitChannel(*cn_data_frame,"CAN_DataFrame.Dir", 8 + 5, 0);
      dir != nullptr) {
    // Add Rx(0) Tx(1) CC block
    if (auto* cc_dir = dir->CreateChannelConversion();
        cc_dir != nullptr) {
      cc_dir->Type(ConversionType::ValueToText);
      cc_dir->Parameter(0, 0.0);
      cc_dir->Parameter(1, 1.0);
      cc_dir->Reference(0, "Rx");
      cc_dir->Reference(1, "Tx");
      cc_dir->Reference(2, ""); // Default text
    }
  }

  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.SRR", 8 + 5, 1);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.EDL", 8 + 5, 2);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.BRS", 8 + 5, 3);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.ESI", 8 + 5, 4);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.WakeUp", 8 + 5, 5);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.SingleWire", 8 + 5, 6);
}

void CanConfigAdapter::CreateCanRemoteFrameChannel(IChannelGroup& group) const {
  auto* cn_remote_frame = group.CreateChannel("CAN_RemoteFrame");
  if (cn_remote_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_DataFrame channel.";
    return;
  }
  cn_remote_frame->Type(ChannelType::FixedLength);
  cn_remote_frame->Sync(ChannelSyncType::None);
  cn_remote_frame->DataType(ChannelDataType::ByteArray);
  cn_remote_frame->DataBytes(6);
  cn_remote_frame->Flags(CnFlag::BusEvent);

  if (auto* frame_bus = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->ByteOffset(8+4);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
    frame_bus->Flags(CnFlag::BusEvent);
  }

  if (auto* frame_id = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.ID");
      frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->ByteOffset(8);
    frame_id->BitOffset(0);
    frame_id->BitCount(29);
    frame_id->Flags(CnFlag::BusEvent);
  }
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.IDE", 8 + 3, 7);

  if (auto* frame_dlc = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.DLC");
      frame_dlc != nullptr) {
    frame_dlc->Type(ChannelType::FixedLength);
    frame_dlc->Sync(ChannelSyncType::None);
    frame_dlc->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_dlc->ByteOffset(8+4);
    frame_dlc->BitOffset(0);
    frame_dlc->BitCount(4);
    frame_dlc->Flags(CnFlag::BusEvent);
  }

  if (auto* frame_length = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->ByteOffset(8+4);
    frame_length->BitOffset(0);
    frame_length->BitCount(4);
    frame_length->Flags(CnFlag::BusEvent);
    if (auto* cc_length = frame_length->CreateChannelConversion();
        cc_length != nullptr) {
      cc_length->Type(ConversionType::ValueToValue);
      uint16_t index = 0;
      for (uint8_t key = 0; key < 16; ++key) {
        cc_length->Parameter(index++,static_cast<double>(key));
        cc_length->Parameter(index++,
                             static_cast<double>(CanMessage::DlcToLength(key)));
      }
    }
  }

  if (auto* dir = CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.Dir", 8 + 5, 0);
      dir != nullptr) {
    // Add Rx(0) Tx(1) CC block
    if (auto* cc_dir = dir->CreateChannelConversion();
        cc_dir != nullptr) {
      cc_dir->Type(ConversionType::ValueToText);
      cc_dir->Parameter(0, 0.0);
      cc_dir->Parameter(1, 1.0);
      cc_dir->Reference(0, "Rx");
      cc_dir->Reference(1, "Tx");
      cc_dir->Reference(2, "");  // Default text
    }
  }
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.SRR", 8 + 5, 1);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.WakeUp", 8 + 5, 5);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.SingleWire", 8 + 5, 6);
}

void CanConfigAdapter::CreateCanErrorFrameChannel(IChannelGroup& group) const {
  auto* cn_error_frame = group.CreateChannel("CAN_ErrorFrame");
  if (cn_error_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_ErrorFrame channel.";
    return;
  }
  cn_error_frame->Type(ChannelType::FixedLength);
  cn_error_frame->Sync(ChannelSyncType::None);
  cn_error_frame->Flags(CnFlag::BusEvent);
  cn_error_frame->DataType(ChannelDataType::ByteArray);

  if (StorageType() == MdfStorageType::MlsdStorage) {
    cn_error_frame->DataBytes(8 + writer_.MaxLength());
  } else {
    cn_error_frame->DataBytes(8 + 8); // Index into SD or VLSD
  }

  if (auto* frame_bus = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->ByteOffset(8+4);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
    frame_bus->Flags(CnFlag::BusEvent);
  }

  if (auto* frame_id = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.ID");
      frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->ByteOffset(8);
    frame_id->BitOffset(0);
    frame_id->BitCount(29);
    frame_id->Flags(CnFlag::BusEvent);
  }
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.IDE", 8 + 3, 7);

  if (auto* frame_dlc = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.DLC");
      frame_dlc != nullptr) {
    frame_dlc->Type(ChannelType::FixedLength);
    frame_dlc->Sync(ChannelSyncType::None);
    frame_dlc->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_dlc->ByteOffset(8+4);
    frame_dlc->BitOffset(0);
    frame_dlc->BitCount(4);
    frame_dlc->Flags(CnFlag::BusEvent);
  }

  if (auto* frame_length = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->ByteOffset(8+4);
    frame_length->BitOffset(0);
    frame_length->BitCount(4);
    frame_length->Flags(CnFlag::BusEvent);
    if (writer_.MaxLength() > 8) {
      if (auto* cc_length = frame_length->CreateChannelConversion();
          cc_length != nullptr) {
        cc_length->Type(ConversionType::ValueToValue);
        uint16_t index = 0;
        for (uint8_t key = 0; key < 16; ++key) {
          cc_length->Parameter(index++,static_cast<double>(key));
          cc_length->Parameter(index++,
                               static_cast<double>(CanMessage::DlcToLength(key)));
        }
        cc_length->Parameter(index, 0.0);
      }
    }
  }

  if (auto* frame_bytes = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.DataBytes");
      frame_bytes != nullptr) {

    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->ByteOffset(8+8);
    frame_bytes->BitOffset(0);
    frame_bytes->Flags(CnFlag::BusEvent);

    switch(StorageType()) {
      case MdfStorageType::MlsdStorage:
        frame_bytes->Type(ChannelType::MaxLength);
        frame_bytes->BitCount(8 * writer_.MaxLength());
        break;

      case MdfStorageType::VlsdStorage:
      default:
        frame_bytes->Type(ChannelType::VariableLength);
        frame_bytes->BitCount(8 * 8); // Index to SD block
        break;
    }
  }

  if (auto* dir = CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.Dir", 8 + 5, 0);
      dir != nullptr) {
    // Add Rx(0) Tx(1) CC block
    if (auto* cc_dir = dir->CreateChannelConversion();
        cc_dir != nullptr) {
      cc_dir->Type(ConversionType::ValueToText);
      cc_dir->Parameter(0, 0.0);
      cc_dir->Parameter(1, 1.0);
      cc_dir->Reference(0, "Rx");
      cc_dir->Reference(1, "Tx");
      cc_dir->Reference(2, "");  // Default text
    }
  }
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.RTR", 8 + 5, 7);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.SRR", 8 + 5, 1);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.EDL", 8 + 5, 2);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.BRS", 8 + 5, 3);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.ESI", 8 + 5, 4);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.WakeUp", 8 + 5, 5);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.SingleWire", 8 + 5, 6);

  if (auto* frame_bit_position = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.BitPosition");
      frame_bit_position != nullptr) {
    frame_bit_position->Type(ChannelType::FixedLength);
    frame_bit_position->Sync(ChannelSyncType::None);
    frame_bit_position->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bit_position->ByteOffset(8+6);
    frame_bit_position->BitOffset(0);
    frame_bit_position->BitCount(8);
    frame_bit_position->Flags(CnFlag::BusEvent);
  }

  if (auto* frame_error_type = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.ErrorType");
      frame_error_type != nullptr) {
    frame_error_type->Type(ChannelType::FixedLength);
    frame_error_type->Sync(ChannelSyncType::None);
    frame_error_type->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_error_type->ByteOffset(8+7);
    frame_error_type->BitOffset(0);
    frame_error_type->BitCount(8);
    frame_error_type->Flags(CnFlag::BusEvent);
  }
}

void CanConfigAdapter::CreateCanOverloadFrameChannel(IChannelGroup& group) {
  auto* cn_overload_frame = group.CreateChannel("CAN_OverloadFrame");
  if (cn_overload_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_OverloadFrame channel.";
    return;
  }
  cn_overload_frame->Type(ChannelType::FixedLength);
  cn_overload_frame->Sync(ChannelSyncType::None);
  cn_overload_frame->DataType(ChannelDataType::ByteArray);
  cn_overload_frame->DataBytes(1);
  cn_overload_frame->Flags(CnFlag::BusEvent);

  if (auto* frame_bus = cn_overload_frame->CreateChannelComposition(
      "CAN_OverloadFrame.BusChannel");
      frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->ByteOffset(8);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
    frame_bus->Flags(CnFlag::BusEvent);
  }

  if (auto* dir = CreateBitChannel(*cn_overload_frame,"CAN_OverloadFrame.Dir", 8 + 0, 0);
      dir != nullptr) {
    // Add Rx(0) Tx(1) CC block
    auto* cc_dir = dir->CreateChannelConversion();
    if (cc_dir != nullptr) {
      cc_dir->Type(ConversionType::ValueToText);
      cc_dir->Parameter(0, 0.0);
      cc_dir->Parameter(1, 1.0);
      cc_dir->Reference(0, "Rx");
      cc_dir->Reference(1, "Tx");
      cc_dir->Reference(2, "");  // Default text
    }
  }

}



}  // namespace mdf