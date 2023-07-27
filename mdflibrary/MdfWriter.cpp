/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */

#include <msclr/marshal_cppstd.h>

#include <string>

#include "MdfWriter.h"
#include "mdflibrary.h"

using namespace msclr::interop;

namespace MdfLibrary {

MdfWriter::MdfWriter(MdfWriterType writer_type) {
  writer_ =
      mdf::MdfFactory::CreateMdfWriterEx(static_cast<mdf::MdfWriterType>(writer_type));
}

MdfWriter::~MdfWriter() { this->!MdfWriter(); }

MdfWriter::!MdfWriter() {
  if (writer_ == nullptr) {
	return;
  }
  delete writer_;
  writer_ = nullptr;
}

MdfFile^ MdfWriter::File::get() {
  auto* mdf_file = writer_ != nullptr
                       ? const_cast<mdf::MdfFile*>(writer_->GetFile())
                       : nullptr;
  return mdf_file != nullptr ? gcnew MdfFile(mdf_file) : nullptr;
}

MdfHeader^ MdfWriter::Header::get() {
  auto* header = writer_ != nullptr
                     ? const_cast<mdf::IHeader*>(writer_->Header())
                     : nullptr;
  return header != nullptr ? gcnew MdfHeader(header) : nullptr;
}

void MdfWriter::CompressData::set(bool compress) {
  if (writer_ != nullptr) 
	writer_->CompressData(compress);
  
}

bool MdfWriter::CompressData::get() {
    auto compress = writer_ != nullptr ? writer_->CompressData() : false;
    return compress; 
}

bool MdfWriter::Init(String^ path_name) {
  return writer_->Init(MdfLibrary::Utf8Conversion(path_name));
}

MdfDataGroup^ MdfWriter::CreateDataGroup() {
  auto* data_group = writer_->CreateDataGroup();
  return data_group != nullptr ? gcnew MdfDataGroup(data_group) : nullptr;
}

bool MdfWriter::InitMeasurement() {
    if (writer_ != nullptr) {
		return writer_->InitMeasurement();
	}
	return false;
}

void MdfWriter::SaveSample(MdfChannelGroup^ group, uint64_t time) {
        if (writer_ != nullptr) {
                writer_->SaveSample(*(group->group_), time);
        }
}

void MdfWriter::StartMeasurement(uint64_t start_time) {
    if (writer_ != nullptr) {
		writer_->StartMeasurement(start_time);
  }
}

void MdfWriter::StopMeasurement(uint64_t stop_time) {
    if (writer_ != nullptr) {
		writer_->StopMeasurement(stop_time);
  }
}

bool MdfWriter::FinalizeMeasurement() {
    if (writer_ != nullptr) {
                return writer_->FinalizeMeasurement();
    }
    return false;
}

}  // namespace MdfLibrary
