/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdfwriter.h"
#include <mdf/idatagroup.h>
#include <mdf/mdflogstream.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>

#include "linconfigadapter.h"
#include "ethconfigadapter.h"
#include "mdfblock.h"
#include "platform.h"
#include "samplequeue.h"
#include "dg4block.h"
#include "dt4block.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

using namespace fs;
using namespace std::chrono_literals;


namespace {
/*
std::string StrErrNo(int error) {
  std::string err_str(200, '\0');
  Platform::strerror(error, err_str.data(), err_str.size());
  return err_str;
}
*/
mdf::IChannel* CreateTimeChannel(mdf::IChannelGroup& group,
                                 const std::string_view& name) {
  auto cn_list = group.Channels();
  // First check if a time channel exist. If so return it unchanged.
  const auto itr = std::find_if(cn_list.begin(),
                                                        cn_list.end(),
                          [] (const auto* channel) {
    return channel != nullptr && channel->Type() == mdf::ChannelType::Master;
  });
  if (itr != cn_list.end()) {
    return *itr;
  }
  auto* time = group.CreateChannel(name.data());
  if (time != nullptr) {
    time->Name(name.data());
    time->Type(mdf::ChannelType::Master);
    time->Sync(mdf::ChannelSyncType::Time);
    time->DataType(mdf::ChannelDataType::FloatLe);
    time->DataBytes(8);
    time->Unit("s");
  }
  return time;
}

mdf::IChannel* CreateBitChannel(mdf::IChannel& parent,
                                const std::string_view& name,
                                uint32_t byte_offset,
                                uint16_t bit_offset) {
  auto* frame_bit = parent.CreateChannelComposition(name);
  if (frame_bit != nullptr) {
    frame_bit->Type(mdf::ChannelType::FixedLength);
    frame_bit->Sync(mdf::ChannelSyncType::None);
    frame_bit->DataType(mdf::ChannelDataType::UnsignedIntegerLe);
    frame_bit->Flags(mdf::CnFlag::BusEvent);
    frame_bit->ByteOffset(byte_offset);
    frame_bit->BitOffset(bit_offset);
    frame_bit->BitCount(1);
  }
  return frame_bit;
}

}  // namespace

namespace mdf {


void MdfWriter::PreTrigTime(double pre_trig_time) {
  pre_trig_time *= 1'000'000'000;
  pre_trig_time_ = static_cast<uint64_t>(pre_trig_time);
}

double MdfWriter::PreTrigTime() const {
  auto temp = static_cast<double>(pre_trig_time_);
  temp /= 1'000'000'000;
  return temp;
}

uint64_t MdfWriter::PreTrigTimeNs() const {
  return pre_trig_time_;
}

IHeader* MdfWriter::Header() const {
  return mdf_file_ ? mdf_file_->Header() : nullptr;
}

IDataGroup* MdfWriter::CreateDataGroup() {
  return !mdf_file_ ? nullptr : mdf_file_->CreateDataGroup();
}

IChannelGroup* MdfWriter::CreateChannelGroup(IDataGroup* parent) {
  return parent == nullptr ? nullptr : parent->CreateChannelGroup();
}

void MdfWriter::Open(std::ios_base::openmode mode) {
  auto* buffer = file_.get();
  if (buffer == nullptr) {
    MDF_ERROR()
    << "Invalid use of function. Need to init the file or stream buffer first.";
    return;
  }
  detail::OpenMdfFile(*file_, filename_, mode);
}

bool MdfWriter::IsOpen() const {
  const auto* buffer = file_.get();
  if (buffer == nullptr) {
    return false;
  }
  try {
    const auto* file = dynamic_cast<const std::filebuf*>(buffer);
    if (file == nullptr) {
      return true;
    }
    return file->is_open();
  } catch (const std::exception& err) {
    MDF_ERROR() << "Checking if file is opened, failed. Error: " << err.what();
  }
  return false;
}

void MdfWriter::Close() {
  auto* buffer = file_.get();
  if (buffer == nullptr) {
    return;
  }
  if (filename_.empty()) {
    // This indicates that the user used the stream buffer interface.
    // Closing the stream buffer may cause that the stream never is
    // opened again.
    buffer->pubsync();
    return;
  }
  try {
    auto* file = dynamic_cast<std::filebuf*>(buffer);
    if (file != nullptr && file->is_open()) {
      file->close();
    } else if (file == nullptr) {
      buffer->pubsync();
    }

  } catch( const std::exception& err) {
    MDF_ERROR() << "Close of file failed. Error: " << err.what();
  }
}

bool MdfWriter::Init(const std::string& filename) {
  bool init = false;
  CreateMdfFile();
  filename_ = filename;
  if (mdf_file_) {
    mdf_file_->FileName(filename);
  }
  file_ = std::make_shared<std::filebuf>();
  if (!file_) {
    MDF_ERROR() << "Failed to create a file buffer. Internal error.";
    return false;
  }
  try {
    if (fs::exists(filename_)) {
      // Read in existing file so we can append to it

      Open(std::ios_base::in | std::ios_base::binary);
      if (IsOpen()) {
        mdf_file_->ReadEverythingButData(*file_);
        Close();
        write_state_ = WriteState::Finalize;  // Append to the file
        MDF_DEBUG() << "Reading existing file. File: " << filename_;
        init = true;
      } else {
        MDF_ERROR() << "Failed to open the existing MDF file. File: "
                    << filename_;
        write_state_ = WriteState::Create;
      }
    } else {
      // Create a new file
      write_state_ = WriteState::Create;  // Indicate the file shall be opened
                                          // with "wb" option.
      init = true;
    }
  } catch (const std::exception& err) {
    if (IsOpen()) {
      Close();
      write_state_ = WriteState::Finalize;
      MDF_ERROR() << "Failed to read the existing MDF file. Error: "
                  << err.what() << ", File: " << filename_;
    } else {
      write_state_ = WriteState::Create;
      MDF_ERROR() << "Failed to open the existing MDF file. Error: "
                  << err.what() << ", File: " << filename_;
    }
  }
  return init;
}

bool MdfWriter::Init(const std::shared_ptr<std::streambuf>& buffer) {
  bool init = true;
  CreateMdfFile();
  file_ = buffer;
  write_state_ = WriteState::Create;
  return init;
}

bool MdfWriter::InitMeasurement() {
  ExitWriteCache();  // Just in case
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  // Set up internal sample buffers so the last channel values can be stored
  const bool prep = PrepareForWriting();
  if (!prep) {
    MDF_ERROR() << "Failed to prepare the file for writing. File: "
                << filename_;
    return false;
  }
  // 1: Save ID, HD, DG, AT, CG and CN blocks to the file.

  Open(write_state_ == WriteState::Create ?
             std::ios_base::out | std::ios_base::binary | std::ios_base::trunc:
             std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }

  const bool write = mdf_file_->Write(*file_);

  SetDataPosition();  // Set up data position to end of file
  Close();
  start_time_ = 0;  // Zero indicate not started
  stop_time_ = 0;   // Zero indicate not stopped
  // Start the working thread that handles the samples
  write_state_ = WriteState::Init;  // Waits for new samples
  InitWriteCache();
  return write;
}

void MdfWriter::StartMeasurement(uint64_t start_time) {
  write_state_ = WriteState::StartMeas;
  start_time_ = start_time;
  stop_time_ = 0;  // Zero indicate not stopped

  // The sample queue actual have absolute time. We have to recalculate the
  // times to relative times by using the start_time_.
  RecalculateTimeMaster();

  NotifySample();

  // Set the time in the header if this is the first DG block in the file.
  // This gives a better start time than when the file was created.
  if ( auto* header = Header();
      header != nullptr && IsFirstMeasurement()) {
    header->StartTime(start_time);
  }

  NotifySample();
}

void MdfWriter::StartMeasurement(ITimestamp &start_time) {
  write_state_ = WriteState::StartMeas;
  start_time_ = start_time.GetUtcTimeNs();
  stop_time_ = 0;  // Zero indicate not stopped
                   
  // The sample queue actual have absolute time. We have to recalculate the
  // times to relative times by using the start_time_.
  RecalculateTimeMaster();

  NotifySample();

  // Set the time in the header if this is the first DG block in the file.
  // This gives a better start time than when the file was created.
  if ( auto* header = Header();
      header != nullptr && IsFirstMeasurement()) {
    header->StartTime(start_time);
  }

  NotifySample();
}

void MdfWriter::StopMeasurement(uint64_t stop_time) {
  ExitWriteCache();
  write_state_ = WriteState::StopMeas;
  stop_time_ = stop_time;
  NotifySample();
}

void MdfWriter::StopMeasurement(ITimestamp& start_time) {
  StopMeasurement(start_time.GetUtcTimeNs());
}

bool MdfWriter::FinalizeMeasurement() {

  // Save outstanding non-written blocks and any block updates as
  // sample counters which changes during DG/DT updates
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  Open(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }
  const bool write = mdf_file_ && mdf_file_->Write(*file_);
  const bool signal_data = WriteSignalData(*file_);
  Close();
  write_state_ = WriteState::Finalize;
  return write && signal_data;
}

IChannel* MdfWriter::CreateChannel(IChannelGroup* parent) {
  return parent == nullptr ? nullptr : parent->CreateChannel();
}

bool MdfWriter::WriteSignalData(std::streambuf&) {
  // Only  supported by MDF4
  return true;
}

std::string MdfWriter::Name() const {
  try {
    auto filename = u8path(filename_).stem().u8string();
    return {filename.begin(), filename.end()};
  } catch (...) {
  }
  return {};
}

bool MdfWriter::CreateBusLogConfiguration() {
  auto* header = Header();
  if (header == nullptr) {
    MDF_ERROR()
        << "File/Header haven't been created yet. Improper use of function";
    return false;
  }

  // 1. Create new DG block if not already exists
  auto* last_dg = header->LastDataGroup();
  if (last_dg == nullptr || !last_dg->IsEmpty()) {
    last_dg = header->CreateDataGroup();
  }
  if (last_dg == nullptr) {
    MDF_ERROR() << "Failed to create a DG block.";
    return false;
  }

  // 2. Create bus dependent CG and CN blocks
  if ((BusType() & MdfBusType::CAN) != 0) {
    CreateCanConfig(*last_dg);
  }
  if ((BusType() & MdfBusType::LIN) != 0) {
    LinConfigAdapter configurator(*this);
    configurator.CreateConfig(*last_dg);
  }
  if ((BusType() & MdfBusType::Ethernet) != 0) {
    EthConfigAdapter configurator(*this);
    configurator.CreateConfig(*last_dg);
  }
  return true;
}

std::string MdfWriter::BusTypeAsString() const {
  std::ostringstream type;

  if ((BusType() & MdfBusType::CAN) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "CAN";
  }

  if ((BusType() & MdfBusType::LIN) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "LIN";
  }

  if ((BusType() & MdfBusType::FlexRay) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "FlexRay";
  }

  if ((BusType() & MdfBusType::MOST) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "MOST";
  }

  if ((BusType() & MdfBusType::Ethernet) != 0) {
    if (!type.str().empty()) {
      type << ",";
    }
    type << "Ethernet";
  }
  return type.str();
}

void MdfWriter::CreateCanConfig(IDataGroup& dg_block) const {
  auto* cg_data_frame = dg_block.CreateChannelGroup("CAN_DataFrame");

  const IChannel* cn_data_byte = nullptr; // Need to update the VLSD Record ID

  if (cg_data_frame != nullptr) {
    cg_data_frame->PathSeparator('.');
    cg_data_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_data_frame,"t");
    CreateCanDataFrameChannel(*cg_data_frame);
    cn_data_byte = cg_data_frame->GetChannel("CAN_DataFrame.DataBytes");
  }

  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples

    auto* cg_samples_frame = dg_block.CreateChannelGroup("");
    if (cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }

  auto* cg_remote_frame = dg_block.CreateChannelGroup("CAN_RemoteFrame");
  if (cg_remote_frame != nullptr) {
    cg_remote_frame->PathSeparator('.');
    cg_remote_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_remote_frame,"t");
    CreateCanRemoteFrameChannel(*cg_remote_frame);
  }

  const IChannel* cn_error_byte = nullptr; // Need to update the VLSD Record ID
  auto* cg_error_frame = dg_block.CreateChannelGroup("CAN_ErrorFrame");
  if (cg_error_frame != nullptr) {
    cg_error_frame->PathSeparator('.');
    cg_error_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_error_frame,"t");
    CreateCanErrorFrameChannel(*cg_error_frame);
    cn_error_byte = cg_error_frame->GetChannel("CAN_ErrorFrame.DataBytes");
  }

  if (StorageType() == MdfStorageType::VlsdStorage && cn_error_byte != nullptr) {
    // Need to add a special CG group for the error samples
    auto* cg_errors_frame = dg_block.CreateChannelGroup("");
    if (cg_errors_frame != nullptr) {
      cg_errors_frame->Flags(CgFlag::VlsdChannel);
      cn_error_byte->VlsdRecordId(cg_errors_frame->RecordId());
    }
  }

  auto* cg_overload_frame = dg_block.CreateChannelGroup("CAN_OverloadFrame");
  if (cg_overload_frame != nullptr) {
    cg_overload_frame->PathSeparator('.');
    cg_overload_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_overload_frame,"t");
    CreateCanOverloadFrameChannel(*cg_overload_frame);
  }


}

void MdfWriter::CreateCanDataFrameChannel(IChannelGroup& group) const {
  auto* cn_data_frame = group.CreateChannel("CAN_DataFrame");
  if (cn_data_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_DataFrame channel.";
    return;
  }

  cn_data_frame->Type(ChannelType::FixedLength);
  cn_data_frame->Sync(ChannelSyncType::None);
  switch (StorageType()) {
    case MdfStorageType::MlsdStorage:
      cn_data_frame->DataBytes(6 + MaxLength());
      break;

    case MdfStorageType::FixedLengthStorage:
    case MdfStorageType::VlsdStorage:
    default:
      cn_data_frame->DataBytes(6 + 8); // Index into SD or VLSD
      break;
  }
  cn_data_frame->Flags(CnFlag::BusEvent);
  cn_data_frame->DataType(ChannelDataType::ByteArray);

  auto* frame_bus = cn_data_frame->CreateChannelComposition("CAN_DataFrame.BusChannel");
  if (frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0,15);
    frame_bus->ByteOffset(8+4);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
  }

  auto* frame_id = cn_data_frame->CreateChannelComposition("CAN_DataFrame.ID");
  if (frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->Flags(CnFlag::BusEvent);
    frame_id->ByteOffset(8);
    frame_id->BitOffset(0);
    frame_id->BitCount(29);
  }
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.IDE", 8 + 3, 7);

  auto* frame_dlc = cn_data_frame->CreateChannelComposition(
      "CAN_DataFrame.DLC");
  if (frame_dlc != nullptr) {
    frame_dlc->Type(ChannelType::FixedLength);
    frame_dlc->Sync(ChannelSyncType::None);
    frame_dlc->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_dlc->Flags(CnFlag::BusEvent);
    frame_dlc->ByteOffset(8+4);
    frame_dlc->BitOffset(0);
    frame_dlc->BitCount(4);
  }

  auto* frame_length = cn_data_frame->CreateChannelComposition(
      "CAN_DataFrame.DataLength");
  if (frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->Flags(CnFlag::BusEvent);
    frame_length->ByteOffset(8+4);
    frame_length->BitOffset(0);
    frame_length->BitCount(4);

    auto* cc_length = frame_length->CreateChannelConversion();
    if (cc_length != nullptr) {
      cc_length->Type(ConversionType::ValueToValue);
      uint16_t index = 0;
      for (uint8_t key = 0; key < 16; ++key) {
        cc_length->Parameter(index++,static_cast<double>(key));
        cc_length->Parameter(index++,
          static_cast<double>(CanMessage::DlcToLength(key)));
      }
    }
  }

  auto* frame_bytes = cn_data_frame->CreateChannelComposition(
      "CAN_DataFrame.DataBytes");
  if (frame_bytes != nullptr) {

    switch(StorageType()) {
      case MdfStorageType::MlsdStorage:
        frame_bytes->Type(ChannelType::MaxLength);
        frame_bytes->BitCount(8 * MaxLength());
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

  auto* dir = CreateBitChannel(*cn_data_frame,"CAN_DataFrame.Dir", 8 + 5, 0);
  if (dir != nullptr) {
    // Add Rx(0) Tx(1) CC block
    auto* cc_dir = dir->CreateChannelConversion();
    if (cc_dir != nullptr) {
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

void MdfWriter::CreateCanRemoteFrameChannel(IChannelGroup& group) const {
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

  auto* frame_bus = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.BusChannel");
  if (frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->ByteOffset(8+4);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
    frame_bus->Flags(CnFlag::BusEvent);
  }

  auto* frame_id = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.ID");
  if (frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->ByteOffset(8);
    frame_id->BitOffset(0);
    frame_id->BitCount(29);
    frame_id->Flags(CnFlag::BusEvent);
  }
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.IDE", 8 + 3, 7);

  auto* frame_dlc = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.DLC");
  if (frame_dlc != nullptr) {
    frame_dlc->Type(ChannelType::FixedLength);
    frame_dlc->Sync(ChannelSyncType::None);
    frame_dlc->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_dlc->ByteOffset(8+4);
    frame_dlc->BitOffset(0);
    frame_dlc->BitCount(4);
    frame_dlc->Flags(CnFlag::BusEvent);
  }

  auto* frame_length = cn_remote_frame->CreateChannelComposition(
      "CAN_RemoteFrame.DataLength");
  if (frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->ByteOffset(8+4);
    frame_length->BitOffset(0);
    frame_length->BitCount(4);
    frame_length->Flags(CnFlag::BusEvent);
    auto* cc_length = frame_length->CreateChannelConversion();
    if (cc_length != nullptr) {
      cc_length->Type(ConversionType::ValueToValue);
      uint16_t index = 0;
      for (uint8_t key = 0; key < 16; ++key) {
        cc_length->Parameter(index++,static_cast<double>(key));
        cc_length->Parameter(index++,
          static_cast<double>(CanMessage::DlcToLength(key)));
      }
    }
  }

  auto* dir = CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.Dir", 8 + 5, 0);
  if (dir != nullptr) {
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
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.SRR", 8 + 5, 1);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.WakeUp", 8 + 5, 5);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.SingleWire", 8 + 5, 6);
}

void MdfWriter::CreateCanErrorFrameChannel(IChannelGroup& group) const {
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
    cn_error_frame->DataBytes(8 + MaxLength());
  } else {
    cn_error_frame->DataBytes(8 + 8); // Index into SD or VLSD
  }

  auto* frame_bus = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.BusChannel");
  if (frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->ByteOffset(8+4);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
    frame_bus->Flags(CnFlag::BusEvent);
  }

  auto* frame_id = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.ID");
  if (frame_id != nullptr) {
    frame_id->Type(ChannelType::FixedLength);
    frame_id->Sync(ChannelSyncType::None);
    frame_id->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_id->ByteOffset(8);
    frame_id->BitOffset(0);
    frame_id->BitCount(29);
    frame_id->Flags(CnFlag::BusEvent);
  }
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.IDE", 8 + 3, 7);

  auto* frame_dlc = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.DLC");
  if (frame_dlc != nullptr) {
    frame_dlc->Type(ChannelType::FixedLength);
    frame_dlc->Sync(ChannelSyncType::None);
    frame_dlc->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_dlc->ByteOffset(8+4);
    frame_dlc->BitOffset(0);
    frame_dlc->BitCount(4);
    frame_dlc->Flags(CnFlag::BusEvent);
  }

  auto* frame_length = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.DataLength");
  if (frame_length != nullptr) {
    frame_length->Type(ChannelType::FixedLength);
    frame_length->Sync(ChannelSyncType::None);
    frame_length->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_length->ByteOffset(8+4);
    frame_length->BitOffset(0);
    frame_length->BitCount(4);
    frame_length->Flags(CnFlag::BusEvent);
    if (MaxLength() > 8) {
      auto* cc_length = frame_length->CreateChannelConversion();
      if (cc_length != nullptr) {
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

  auto* frame_bytes = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.DataBytes");
  if (frame_bytes != nullptr) {

    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->ByteOffset(8+8);
    frame_bytes->BitOffset(0);
    frame_bytes->Flags(CnFlag::BusEvent);

    switch(StorageType()) {
      case MdfStorageType::MlsdStorage:
        frame_bytes->Type(ChannelType::MaxLength);
        frame_bytes->BitCount(8 * MaxLength());
        break;

      case MdfStorageType::VlsdStorage:
      default:
        frame_bytes->Type(ChannelType::VariableLength);
        frame_bytes->BitCount(8 * 8); // Index to SD block
        break;
    }
  }

  auto* dir = CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.Dir", 8 + 5, 0);
  if (dir != nullptr) {
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
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.RTR", 8 + 5, 7);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.SRR", 8 + 5, 1);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.EDL", 8 + 5, 2);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.BRS", 8 + 5, 3);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.ESI", 8 + 5, 4);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.WakeUp", 8 + 5, 5);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.SingleWire", 8 + 5, 6);

  auto* frame_bit_position = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.BitPosition");
  if (frame_bit_position != nullptr) {
    frame_bit_position->Type(ChannelType::FixedLength);
    frame_bit_position->Sync(ChannelSyncType::None);
    frame_bit_position->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bit_position->ByteOffset(8+6);
    frame_bit_position->BitOffset(0);
    frame_bit_position->BitCount(8);
    frame_bit_position->Flags(CnFlag::BusEvent);
  }

  auto* frame_error_type = cn_error_frame->CreateChannelComposition(
      "CAN_ErrorFrame.ErrorType");
  if (frame_error_type != nullptr) {
    frame_error_type->Type(ChannelType::FixedLength);
    frame_error_type->Sync(ChannelSyncType::None);
    frame_error_type->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_error_type->ByteOffset(8+7);
    frame_error_type->BitOffset(0);
    frame_error_type->BitCount(8);
    frame_error_type->Flags(CnFlag::BusEvent);
  }
}

void MdfWriter::CreateCanOverloadFrameChannel(IChannelGroup& group) {
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

  auto* frame_bus = cn_overload_frame->CreateChannelComposition(
      "CAN_OverloadFrame.BusChannel");
  if (frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->ByteOffset(8);
    frame_bus->BitOffset(4);
    frame_bus->BitCount(4);
    frame_bus->Flags(CnFlag::BusEvent);
  }
  auto* dir = CreateBitChannel(*cn_overload_frame,"CAN_OverloadFrame.Dir", 8 + 0, 0);
  if (dir != nullptr) {
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
bool MdfWriter::IsFirstMeasurement() const {
  const auto* header = Header();
  if (header == nullptr) {
    return true;
  }

  for (const auto* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (const auto* channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||
          (channel_group->Flags() & CgFlag::VlsdChannel) != 0) {
        continue;
      }
      if (channel_group->NofSamples() > 0) {
        return false;
      }
    }

  }
  return true;
}

void MdfWriter::SetDataPosition() {
  if (CompressData() || !file_) {
    return;
  }

  const auto* header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No MDF header. Invalid use of function.";
    return;
  }

  std::vector<IDataGroup*> measurement_list;

  for (IDataGroup* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (IChannelGroup* channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||
          (channel_group->Flags() & CgFlag::VlsdChannel) != 0 ||
          channel_group->NofSamples() > 0) {
        continue;
      }
      measurement_list.push_back(data_group);
      break;
    }
  }

  for (IDataGroup* measurement : measurement_list) {
    if (measurement == nullptr) {
      continue;
    }

    auto* dg4 = dynamic_cast<detail::Dg4Block*>(measurement);
    if (dg4 == nullptr) {
      return;
    }
    auto& block_list = dg4->DataBlockList();
    if (block_list.empty()) {
      return;
    }
    auto* dt4 = dynamic_cast<detail::Dt4Block*>(block_list.back().get());
    if (dt4 == nullptr) {
      return;
    }

    file_->pubseekoff(0, std::ios_base::end);

    if (dg4->Link(2) > 0) {
      return;
    }

    dg4->SetLastFilePosition(*file_);
    const auto position = detail::GetFilePosition(*file_);
    dg4->UpdateLink(*file_, 2, position);
    dg4->SetLastFilePosition(*file_);

    const int64_t data_position = detail::GetFilePosition(*file_);
    dt4->DataPosition(data_position);

  }
}

/** \brief Checks if the compression is required
 *
 * The function checks if the compression is required to be enabled.
 * This is required if it is MDF 4 and more than one DG block is active.
 */


}  // namespace mdf
