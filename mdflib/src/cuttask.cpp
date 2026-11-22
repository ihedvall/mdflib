/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "cuttask.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <memory>
#include <deque>

#include "copysampleobserver.h"
#include "mdf/dgcomment.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdffactory.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/ifilehistory.h"
#include "mdf/fhcomment.h"
#include "mdf/mdproperty.h"

using namespace std::filesystem;

namespace mdf {

void CutTask::Run() {
  try {
    Result(false);
    CheckSourceFile();
    CheckDestinationFile();
    CreateDestinationTempFile();
    CheckSourceAndDestinationDiff();
    ReadConfig();
    CreateWriter();
    CutFile();
    CopyTempFile();
    DeleteTempFile();
    Error(false);
    Result(true);
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Failed to run the sorting task. Error: " << err.what()
        << ", Source: " << SourceFile();
    MDF_ERROR() << oss.str();
    Error(true);
    SaveMessage(oss.str());
  }
}

void CutTask::CutFile() {
  const std::string& source_path = SourceFile();
  const std::string& destination_path = DestinationFile();
  const std::string& temp_path = TempFile();
  try {
    if (source_path.empty()
       || destination_path.empty()
       || temp_path.empty()) {
      throw std::runtime_error("Invalid (empty) file names.");
    }
    if (!reader_ || !writer_) {
      throw std::runtime_error("Reader or writer is not initialized.");
    }
    // The configuration copy only handles MDF 4 blocks
    if (!reader_->GetFile()->IsMdf4()) {
      throw std::runtime_error("MDF 3 source files are not supported.");
    }
    // The writer normally calculate bit and byte offsets for each channel.
    // As we are using the sample observer, the bit and byte offsets have to be
    // the same as in the source file.
    // We need to turn off the automatic offset calculation.
    writer_->CalculateBitAndByteOffsets(false);
    CopyMainConfig();

    const IHeader* source_header = reader_->GetHeader();
    if (source_header == nullptr) {
      throw std::invalid_argument("Failed to get the source header. File: "
        + SourceFile());
    }
    start_time_ = source_header->StartTime();

    IHeader* dest_header = writer_->Header();
    if (dest_header == nullptr) {
      throw std::invalid_argument("Failed to get the destination header. File: "
        + TempFile());
    }

    AppendHistory();

    const auto source_dg_list = source_header->DataGroups();
    for (IDataGroup* source_dg : source_dg_list) {
      if (source_dg == nullptr) {
        continue;
      }

      std::deque<std::unique_ptr<CopySampleObserver>> observer_list;
      IDataGroup* dest_dg = dest_header->CreateDataGroup();
      if (dest_dg == nullptr) {
        throw std::logic_error(
          "Failed to create the destination data group. File: "
            + TempFile());
      }

      if (source_dg->MetaData() != nullptr) {
        DgComment comment;
        source_dg->GetDgComment(comment);
        dest_dg->SetDgComment(comment);
      }
      const auto source_cg_list = source_dg->ChannelGroups();
      for (const IChannelGroup* source_cg : source_cg_list) {
        if (source_cg == nullptr) {
          continue;
        }
        if ((source_cg->Flags() & CgFlag::VlsdChannel) != 0) {
          // The data should be stored in a SD block
          continue;
        }
        if (SkipIfNoSamples() && source_cg->NofSamples() == 0) {
          continue;
        }
        IChannelGroup* dest_cg = CopyChannelConfig(*source_cg, *dest_dg);
        if (dest_cg == nullptr) {
          throw std::logic_error(
            "Failed to copy the channel group configuration. Channel Group: "
              + source_cg->Name());
        }
        if (source_cg->NofSamples() > 0) {
          auto observer = std::make_unique<CopySampleObserver>(
            *source_dg, *source_cg,
              start_time_, *writer_,
              *dest_dg, *dest_cg);
          if (observer) {
            observer->SetTimeRange(MinTime(), MaxTime());
            observer_list.emplace_back(std::move(observer));
          }

        }
      } // End of channel group list

      const bool init_meas = writer_->InitMeasurement();
      if (!init_meas) {
        throw std::runtime_error(
          "Failed to initialize the measurement. File: " + TempFile());
      }
      uint64_t start_time = start_time_;
      if (RecalculateTime()) {
        const auto time_offset = static_cast<int64_t>(MinTime() * 1'000'000'000);
        start_time += time_offset;
      }
      // Keep the time zone of the source file. The numeric start time would
      // get the time zone of this computer.
      const auto* source_time = source_header->StartTimestamp();
      if (source_time != nullptr &&
          source_time->GetTimeType() == timetype::kTimezoneTime) {
        TimezoneTimestamp timestamp(
            start_time, static_cast<int16_t>(source_time->GetTzOffsetMin()),
            static_cast<int16_t>(source_time->GetDstOffsetMin()));
        writer_->StartMeasurement(timestamp);
      } else {
        writer_->StartMeasurement(start_time);
      }
      reader_->ReadData(*source_dg);
      uint64_t stop_time = start_time;
      for (auto& sample_observer : observer_list) {
        if (sample_observer && sample_observer->GetSampleTime() > stop_time) {
          stop_time = sample_observer->GetSampleTime();
        }
      }
      writer_->StopMeasurement(stop_time);

      const bool finalize = writer_->FinalizeMeasurement();
      if (!finalize) {
        throw std::runtime_error(
          "Failed to finalize the measurement. File: " + TempFile());
      }
    }
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Failed to sort the file. Error: " << err.what();
    Error(true);
    throw std::runtime_error(oss.str());
  }

}

void CutTask::ReadInData(IDataGroup& data_group, const IChannelGroup& channel_group) {
  if (!reader_) {
    throw std::runtime_error("Reader is not initialized.");
  }
  observer_list_.clear();
  CreateObserverForTopLevelChannels(data_group, channel_group, observer_list_);
  if (observer_list_.empty()) {
    // No meaning to reading the data
    return;
  }
  if (const bool read = reader_->ReadData(data_group); !read) {
    throw std::runtime_error("Failed to read the data.");
  }
  start_time_ = reader_->GetStartTime();
  sample_time_ = start_time_;
}

void CutTask::CopyData(const IChannelGroup& source_cg,
                           const IChannelGroup& dest_cg) {
  if (!writer_) {
    throw std::runtime_error("Writer is not initialized.");
  }

  sample_time_ = start_time_;
  for (uint64_t sample = 0; sample < source_cg.NofSamples(); ++sample) {
    sample_time_ = start_time_ + sample; // In case of no master relative time
    for (auto& observer : observer_list_) {
      if (!observer) {
        continue;
      }
      const IChannel& source_channel = observer->Channel();
      if (source_channel.Type() == ChannelType::Master &&
          source_channel.Sync() == ChannelSyncType::Time) {
        double relative_time = 0;
        observer->GetEngValue(sample, relative_time);
        sample_time_ = start_time_ +
          static_cast<int64_t>(relative_time * 1'000'000'000 );
        continue;
      }
      IChannel* dest_channel = dest_cg.GetChannel(source_channel.Name());
      if (dest_channel == nullptr) {
        continue;
      }
      const uint64_t array_size = dest_channel->ArraySize();
      switch (dest_channel->DataType()) {
        case ChannelDataType::UnsignedIntegerLe:
        case ChannelDataType::UnsignedIntegerBe:
           for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            uint64_t unsigned_value = 0;
            const bool valid = observer->GetChannelValue(sample,
               unsigned_value, array_index);
            dest_channel->SetChannelValue(unsigned_value, valid, array_index);
          }
          break;

        case ChannelDataType::SignedIntegerLe:
        case ChannelDataType::SignedIntegerBe:
           for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            int64_t signed_value = 0;
            const bool valid = observer->GetChannelValue(sample,
               signed_value, array_index);
            dest_channel->SetChannelValue(signed_value, valid, array_index);
          }
          break;

        case ChannelDataType::FloatLe:
        case ChannelDataType::FloatBe:
           for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            double float_value = 0.0;
            const bool valid = observer->GetChannelValue(sample,
               float_value, array_index);
            dest_channel->SetChannelValue(float_value, valid, array_index);
          }
          break;

        case ChannelDataType::CanOpenDate:
        case ChannelDataType::CanOpenTime:
        case ChannelDataType::MimeStream:
        case ChannelDataType::MimeSample:
        case ChannelDataType::ByteArray:
          for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            std::vector<uint8_t> array_value;
            const bool valid = observer->GetChannelValue(sample,
               array_value, array_index);
            dest_channel->SetChannelValue(array_value, valid, array_index);
          }
          break;

        default:
          for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            std::string text_value;
            const bool valid = observer->GetChannelValue(sample,
               text_value, array_index);
            dest_channel->SetChannelValue(text_value, valid, array_index);
          }
          break;

      }
    }
    writer_->SaveSample(dest_cg, sample_time_);
  }

}

void CutTask::AppendHistory() const {
  if (!writer_) {
    throw std::runtime_error("Writer is not initialized.");
  }

  IHeader* dest_header = writer_->Header();
  if (dest_header == nullptr) {
    throw std::invalid_argument("Failed to get the destination header. File: "
      + TempFile());
  }

  // Add extra FH block that have info about this cutting task.
  IFileHistory* cut_history = dest_header->CreateFileHistory();
  if (cut_history == nullptr) {
    throw std::runtime_error("Failed to create the MDF file history.");
  }
  cut_history->Time(MdfHelper::NowNs());

  FhComment fh_comment("Changed Time Range");
  {
    MdProperty min_prop;
    min_prop.Name("Min Time");
    min_prop.DataType(MdDataType::MdFloat);
    min_prop.Value(MinTime());
    min_prop.Unit("s");
    fh_comment.AddProperty(min_prop);
  }

  {
    MdProperty max_prop;
    max_prop.Name("Max Time");
    max_prop.DataType(MdDataType::MdFloat);
    max_prop.Value(MaxTime());
    max_prop.Unit("s");
    fh_comment.AddProperty(max_prop);
  }

  {
    MdProperty recalc_prop;
    recalc_prop.Name("Recalculate Time");
    recalc_prop.DataType(MdDataType::MdBoolean);
    recalc_prop.Value(RecalculateTime());
    fh_comment.AddProperty(recalc_prop);
  }

  {
    MdProperty orig_prop;
    orig_prop.Name("Original File");
    orig_prop.DataType(MdDataType::MdString);
    orig_prop.Value(SourceFile());
    fh_comment.AddProperty(orig_prop);
  }

  cut_history->SetFhComment(fh_comment);
}

} // mdf