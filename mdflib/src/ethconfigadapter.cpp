/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/ethconfigadapter.h"

#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdflogstream.h"

namespace mdf {
EthConfigAdapter::EthConfigAdapter(const MdfWriter& writer)
    : IConfigAdapter(writer) {
  BusType(MdfBusType::Ethernet);
  BusName("ETH");
}

void EthConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  const IChannel* cn_data_byte = nullptr; // Need to update the VLSD Record ID
  if (IChannelGroup* cg_frame = dg_block.CreateChannelGroup("ETH_Frame");
      cg_frame != nullptr) {
    cg_frame->PathSeparator('.');
    cg_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_frame);
    CreateTimeChannel(*cg_frame, "t");
    CreateFrameChannels(*cg_frame);
    cn_data_byte = cg_frame->GetChannel("ETH_Frame.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
      cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_samples = dg_block.CreateChannelGroup("Samples");
     cg_samples != nullptr) {
      cg_samples->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples->RecordId());
    }
  }

  cn_data_byte = nullptr;
  if (IChannelGroup* cg_checksum_error = dg_block.CreateChannelGroup("ETH_ChecksumError");
      cg_checksum_error != nullptr) {
    cg_checksum_error->PathSeparator('.');
    cg_checksum_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_checksum_error);
    CreateTimeChannel(*cg_checksum_error, "t");
    CreateChecksumErrorChannels(*cg_checksum_error);
    // Note that the cn_data_byte not is mandatory
    cn_data_byte = cg_checksum_error->GetChannel("ETH_ChecksumError.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
      cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_errors = dg_block.CreateChannelGroup("ChecksumErrors");
        cg_errors != nullptr) {
      cg_errors->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_errors->RecordId());
    }
  }

  cn_data_byte = nullptr;
  if (IChannelGroup* cg_length_error = dg_block.CreateChannelGroup("ETH_LengthError");
     cg_length_error != nullptr) {
   cg_length_error->PathSeparator('.');
   cg_length_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
   CreateSourceInformation(*cg_length_error);
   CreateTimeChannel(*cg_length_error, "t");
   CreateLengthErrorChannels(*cg_length_error);
   // Note that the cn_data_byte not is mandatory
   cn_data_byte = cg_length_error->GetChannel("ETH_LengthError.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
    cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_errors = dg_block.CreateChannelGroup("LengthErrors");
       cg_errors != nullptr) {
     cg_errors->Flags(CgFlag::VlsdChannel);
     cn_data_byte->VlsdRecordId(cg_errors->RecordId());
    }
  }

  cn_data_byte = nullptr;
  if (IChannelGroup* cg_receive_error = dg_block.CreateChannelGroup("ETH_ReceiveError");
      cg_receive_error != nullptr) {
    cg_receive_error->PathSeparator('.');
    cg_receive_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_receive_error);
    CreateTimeChannel(*cg_receive_error, "t");
    CreateReceiveErrorChannels(*cg_receive_error);
    // Note that the cn_data_byte not is mandatory
    cn_data_byte = cg_receive_error->GetChannel("ETH_ReceiveError.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
      cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_errors = dg_block.CreateChannelGroup("ReceiveErrors");
        cg_errors != nullptr) {
      cg_errors->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_errors->RecordId());
    }
  }

}

void EthConfigAdapter::CreateFrameChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 27 : 33);
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
    frame_bus->BitCount(4);
  }
  if (IChannel* dir = CreateBitChannel(*cn_frame, group_name +".Dir", 8 + 0, 7);
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

  if (IChannel* source =
          cn_frame->CreateChannelComposition(group_name + ".Source");
      source != nullptr) {
    source->Type(ChannelType::FixedLength);
    source->Sync(ChannelSyncType::None);
    source->DataType(ChannelDataType::ByteArray);
    source->Flags(CnFlag::BusEvent);
    source->ByteOffset(8 + 1);
    source->BitOffset(0);
    source->BitCount(48);
  }

  if (IChannel* destination =
          cn_frame->CreateChannelComposition(group_name + ".Destination");
      destination != nullptr) {
    destination->Type(ChannelType::FixedLength);
    destination->Sync(ChannelSyncType::None);
    destination->DataType(ChannelDataType::ByteArray);
    destination->Flags(CnFlag::BusEvent);
    destination->ByteOffset(8 + 7);
    destination->BitOffset(0);
    destination->BitCount(48);
  }

  if (IChannel* type =
          cn_frame->CreateChannelComposition(group_name + ".EthType");
      type != nullptr) {
    type->Type(ChannelType::FixedLength);
    type->Sync(ChannelSyncType::None);
    type->DataType(ChannelDataType::UnsignedIntegerLe);
    type->Flags(CnFlag::BusEvent);
    type->ByteOffset(8 + 13);
    type->BitOffset(0);
    type->BitCount(16);
  }

  if (IChannel* nof_rec =
          cn_frame->CreateChannelComposition(group_name + ".ReceivedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 15);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(16);
  }

  if (IChannel* frame_length =
          cn_frame->CreateChannelComposition(group_name + ".DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8 + 17);
    frame_length->BitOffset(0);
    frame_length->BitCount(16);
  }

  // In reality the VSLD-CG storage is the only possible solution. The
  // SD storage is possible in theory only.
  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition(group_name + ".DataBytes");
      frame_bytes != nullptr) {
     frame_bytes->Type(ChannelType::VariableLength);
    frame_bytes->BitCount(8 * 8);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(8 + 19);
    frame_bytes->BitOffset(0);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* crc = cn_frame->CreateChannelComposition(group_name + ".CRC");
      crc != nullptr) {
    crc->Type(ChannelType::FixedLength);
    crc->Sync(ChannelSyncType::None);
    crc->DataType(ChannelDataType::UnsignedIntegerLe);
    crc->Flags(CnFlag::BusEvent);
    crc->ByteOffset(8 + 27);
    crc->BitOffset(0);
    crc->BitCount(32);
  }
  if (IChannel* padding_bytes =
          cn_frame->CreateChannelComposition(group_name + ".PadByteCount");
      padding_bytes != nullptr) {
    padding_bytes->Type(ChannelType::FixedLength);
    padding_bytes->Sync(ChannelSyncType::None);
    padding_bytes->DataType(ChannelDataType::UnsignedIntegerLe);
    padding_bytes->Flags(CnFlag::BusEvent);
    padding_bytes->ByteOffset(8 + 31);
    padding_bytes->BitOffset(0);
    padding_bytes->BitCount(16);
  }
 }


void EthConfigAdapter::CreateChecksumErrorChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 25 : 37);
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
    frame_bus->BitCount(4);
  }

  if (IChannel* dir = CreateBitChannel(*cn_frame, group_name +".Dir", 8 + 0, 7);
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

  if (IChannel* source =
          cn_frame->CreateChannelComposition(group_name + ".Source");
      source != nullptr) {
    source->Type(ChannelType::FixedLength);
    source->Sync(ChannelSyncType::None);
    source->DataType(ChannelDataType::ByteArray);
    source->Flags(CnFlag::BusEvent);
    source->ByteOffset(8 + 1);
    source->BitOffset(0);
    source->BitCount(48);
  }

  if (IChannel* destination =
          cn_frame->CreateChannelComposition(group_name + ".Destination");
      destination != nullptr) {
    destination->Type(ChannelType::FixedLength);
    destination->Sync(ChannelSyncType::None);
    destination->DataType(ChannelDataType::ByteArray);
    destination->Flags(CnFlag::BusEvent);
    destination->ByteOffset(8 + 7);
    destination->BitOffset(0);
    destination->BitCount(48);
  }

  if (IChannel* type =
          cn_frame->CreateChannelComposition(group_name + ".EthType");
      type != nullptr) {
    type->Type(ChannelType::FixedLength);
    type->Sync(ChannelSyncType::None);
    type->DataType(ChannelDataType::UnsignedIntegerLe);
    type->Flags(CnFlag::BusEvent);
    type->ByteOffset(8 + 13);
    type->BitOffset(0);
    type->BitCount(16);
  }

  if (IChannel* frame_length =
          cn_frame->CreateChannelComposition(group_name + ".DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8 + 15);
    frame_length->BitOffset(0);
    frame_length->BitCount(16);
  }

  if (IChannel* crc = cn_frame->CreateChannelComposition(group_name + ".CRC");
      crc != nullptr) {
    crc->Type(ChannelType::FixedLength);
    crc->Sync(ChannelSyncType::None);
    crc->DataType(ChannelDataType::UnsignedIntegerLe);
    crc->Flags(CnFlag::BusEvent);
    crc->ByteOffset(8 + 17);
    crc->BitOffset(0);
    crc->BitCount(32);
  }

  if (IChannel* expected_crc = cn_frame->CreateChannelComposition(group_name + ".ExpectedCRC");
      expected_crc != nullptr) {
    expected_crc->Type(ChannelType::FixedLength);
    expected_crc->Sync(ChannelSyncType::None);
    expected_crc->DataType(ChannelDataType::UnsignedIntegerLe);
    expected_crc->Flags(CnFlag::BusEvent);
    expected_crc->ByteOffset(8 + 21);
    expected_crc->BitOffset(0);
    expected_crc->BitCount(32);
  }

  if (mandatory) {
    return;
  }

  // In reality the VSLD-CG storage is the only possible solution. The
  // SD storage is possible in theory only.
  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition(group_name + ".DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::VariableLength);
    frame_bytes->BitCount(8 * 8);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(8 + 25);
    frame_bytes->BitOffset(0);
  }

  if (IChannel* nof_rec =
          cn_frame->CreateChannelComposition(group_name + ".ReceivedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 33);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(16);
  }


  if (IChannel* padding_bytes =
          cn_frame->CreateChannelComposition(group_name + ".PadByteCount");
      padding_bytes != nullptr) {
    padding_bytes->Type(ChannelType::FixedLength);
    padding_bytes->Sync(ChannelSyncType::None);
    padding_bytes->DataType(ChannelDataType::UnsignedIntegerLe);
    padding_bytes->Flags(CnFlag::BusEvent);
    padding_bytes->ByteOffset(8 + 35);
    padding_bytes->BitOffset(0);
    padding_bytes->BitCount(16);
  }

}

void EthConfigAdapter::CreateLengthErrorChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 17 : 33);
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
    frame_bus->BitCount(4);
  }
  if (IChannel* dir = CreateBitChannel(*cn_frame, group_name +".Dir", 8 + 0, 7);
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

  if (IChannel* source =
          cn_frame->CreateChannelComposition(group_name + ".Source");
      source != nullptr) {
    source->Type(ChannelType::FixedLength);
    source->Sync(ChannelSyncType::None);
    source->DataType(ChannelDataType::ByteArray);
    source->Flags(CnFlag::BusEvent);
    source->ByteOffset(8 + 1);
    source->BitOffset(0);
    source->BitCount(48);
  }

  if (IChannel* destination =
          cn_frame->CreateChannelComposition(group_name + ".Destination");
      destination != nullptr) {
    destination->Type(ChannelType::FixedLength);
    destination->Sync(ChannelSyncType::None);
    destination->DataType(ChannelDataType::ByteArray);
    destination->Flags(CnFlag::BusEvent);
    destination->ByteOffset(8 + 7);
    destination->BitOffset(0);
    destination->BitCount(48);
  }

  if (IChannel* type =
          cn_frame->CreateChannelComposition(group_name + ".EthType");
      type != nullptr) {
    type->Type(ChannelType::FixedLength);
    type->Sync(ChannelSyncType::None);
    type->DataType(ChannelDataType::UnsignedIntegerLe);
    type->Flags(CnFlag::BusEvent);
    type->ByteOffset(8 + 13);
    type->BitOffset(0);
    type->BitCount(16);
  }

  if (IChannel* nof_rec =
          cn_frame->CreateChannelComposition(group_name + ".ReceivedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 15);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(16);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* frame_length =
          cn_frame->CreateChannelComposition(group_name + ".DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8 + 17);
    frame_length->BitOffset(0);
    frame_length->BitCount(16);
  }

  // In reality the VSLD-CG storage is the only possible solution. The
  // SD storage is possible in theory only.
  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition(group_name + ".DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::VariableLength);
    frame_bytes->BitCount(8 * 8);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(8 + 19);
    frame_bytes->BitOffset(0);
  }


  if (IChannel* crc = cn_frame->CreateChannelComposition(group_name + ".CRC");
      crc != nullptr) {
    crc->Type(ChannelType::FixedLength);
    crc->Sync(ChannelSyncType::None);
    crc->DataType(ChannelDataType::UnsignedIntegerLe);
    crc->Flags(CnFlag::BusEvent);
    crc->ByteOffset(8 + 27);
    crc->BitOffset(0);
    crc->BitCount(32);
  }

  if (IChannel* padding_bytes =
          cn_frame->CreateChannelComposition(group_name + ".PadByteCount");
      padding_bytes != nullptr) {
    padding_bytes->Type(ChannelType::FixedLength);
    padding_bytes->Sync(ChannelSyncType::None);
    padding_bytes->DataType(ChannelDataType::UnsignedIntegerLe);
    padding_bytes->Flags(CnFlag::BusEvent);
    padding_bytes->ByteOffset(8 + 31);
    padding_bytes->BitOffset(0);
    padding_bytes->BitCount(16);
  }
}

void EthConfigAdapter::CreateReceiveErrorChannels(IChannelGroup& group) {
  const bool mandatory = writer_.MandatoryMembersOnly();
  const std::string group_name = group.Name();
  auto* cn_frame = group.CreateChannel(group_name);
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << group_name;
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 17 : 33);
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
    frame_bus->BitCount(4);
  }

  if (IChannel* dir = CreateBitChannel(*cn_frame, group_name +".Dir", 8 + 0, 7);
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

  if (IChannel* error_type =
          cn_frame->CreateChannelComposition(group_name + ".ErrorType");
      error_type != nullptr) {
    error_type->Type(ChannelType::FixedLength);
    error_type->Sync(ChannelSyncType::None);
    error_type->DataType(ChannelDataType::UnsignedIntegerLe);
    error_type->Flags(CnFlag::BusEvent);
    error_type->ByteOffset(8 + 0);
    error_type->BitOffset(4);
    error_type->BitCount(3);
    if (IChannelConversion* cc_type = error_type->CreateChannelConversion();
        cc_type != nullptr) {
      cc_type->Type(ConversionType::ValueToText);
      cc_type->Parameter(0, 0.0);
      cc_type->Parameter(1, 1.0);
      cc_type->Reference(0, "Unknown");
      cc_type->Reference(1, "Collision Error");
      cc_type->Reference(2, "");  // Default text
    }
  }

  if (IChannel* source =
          cn_frame->CreateChannelComposition(group_name + ".Source");
      source != nullptr) {
    source->Type(ChannelType::FixedLength);
    source->Sync(ChannelSyncType::None);
    source->DataType(ChannelDataType::ByteArray);
    source->Flags(CnFlag::BusEvent);
    source->ByteOffset(8 + 1);
    source->BitOffset(0);
    source->BitCount(6 * 8);
  }

  if (IChannel* destination =
          cn_frame->CreateChannelComposition(group_name + ".Destination");
      destination != nullptr) {
    destination->Type(ChannelType::FixedLength);
    destination->Sync(ChannelSyncType::None);
    destination->DataType(ChannelDataType::ByteArray);
    destination->Flags(CnFlag::BusEvent);
    destination->ByteOffset(8 + 7);
    destination->BitOffset(0);
    destination->BitCount(6*8);
  }

  if (IChannel* type =
          cn_frame->CreateChannelComposition(group_name + ".EthType");
      type != nullptr) {
    type->Type(ChannelType::FixedLength);
    type->Sync(ChannelSyncType::None);
    type->DataType(ChannelDataType::UnsignedIntegerLe);
    type->Flags(CnFlag::BusEvent);
    type->ByteOffset(8 + 13);
    type->BitOffset(0);
    type->BitCount(16);
  }

  if (IChannel* nof_rec =
          cn_frame->CreateChannelComposition(group_name + ".ReceivedDataByteCount");
      nof_rec != nullptr) {
    nof_rec->Type(ChannelType::FixedLength);
    nof_rec->Sync(ChannelSyncType::None);
    nof_rec->DataType(ChannelDataType::UnsignedIntegerLe);
    nof_rec->Flags(CnFlag::BusEvent);
    nof_rec->ByteOffset(8 + 15);
    nof_rec->BitOffset(0);
    nof_rec->BitCount(16);
  }

  if (mandatory) {
    return;
  }

  if (IChannel* frame_length =
          cn_frame->CreateChannelComposition(group_name + ".DataLength");
      frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8 + 17);
    frame_length->BitOffset(0);
    frame_length->BitCount(16);
  }

  // In reality the VSLD-CG storage is the only possible solution. The
  // SD storage is possible in theory only.
  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition(group_name + ".DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::VariableLength);
    frame_bytes->BitCount(8 * 8);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(8 + 19);
    frame_bytes->BitOffset(0);
  }


  if (IChannel* crc = cn_frame->CreateChannelComposition(group_name + ".CRC");
      crc != nullptr) {
    crc->Type(ChannelType::FixedLength);
    crc->Sync(ChannelSyncType::None);
    crc->DataType(ChannelDataType::UnsignedIntegerLe);
    crc->Flags(CnFlag::BusEvent);
    crc->ByteOffset(8 + 27);
    crc->BitOffset(0);
    crc->BitCount(32);
  }

  if (IChannel* padding_bytes =
          cn_frame->CreateChannelComposition(group_name + ".PadByteCount");
      padding_bytes != nullptr) {
    padding_bytes->Type(ChannelType::FixedLength);
    padding_bytes->Sync(ChannelSyncType::None);
    padding_bytes->DataType(ChannelDataType::UnsignedIntegerLe);
    padding_bytes->Flags(CnFlag::BusEvent);
    padding_bytes->ByteOffset(8 + 31);
    padding_bytes->BitOffset(0);
    padding_bytes->BitCount(16);
  }
}

}  // namespace mdf