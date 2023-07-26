/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>
#include "MdfHeader.h"
#include "mdflibrary.h"

using namespace msclr::interop;

namespace MdfLibrary {

int64_t MdfHeader::Index::get() {
  return header_ != nullptr ? header_->Index() : 0; 
}

MdfHeader::MdfHeader(mdf::IHeader* header)
  : header_(header) {
}

String^ MdfHeader::Description::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->Description()) : gcnew String("");
}

void MdfHeader::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
    std::string() : marshal_as<std::string>(desc);
  if (header_ != nullptr) {
    header_->Description(temp);
  }    
}

String^ MdfHeader::Author::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->Author()) : gcnew String("");
}

void MdfHeader::Author::set(String^ author) {
  const auto temp = String::IsNullOrEmpty(author) ?
    std::string() : marshal_as<std::string>(author);
  if (header_ != nullptr) {
    header_->Author(temp);
  }      
}

String^ MdfHeader::Department::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->Department()) : gcnew String("");  
}

void MdfHeader::Department::set(String^ department) {
  const auto temp = String::IsNullOrEmpty(department) ?
    std::string() : marshal_as<std::string>(department);
  if (header_ != nullptr) {
    header_->Department(temp);
  }   
}

String^ MdfHeader::Project::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->Project()) : gcnew String("");    
}

void MdfHeader::Project::set(String^ project) {
  if (header_ != nullptr) {
    header_->Project(String::IsNullOrEmpty(project) ?
      std::string() : marshal_as<std::string>(project));
  }     
}

String^ MdfHeader::Subject::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->Subject()) : gcnew String("");   
}

void MdfHeader::Subject::set(String^ subject) {
  if (header_ != nullptr) {
    header_->Subject(String::IsNullOrEmpty(subject) ?
      std::string() : marshal_as<std::string>(subject));
  }    
}

String^ MdfHeader::MeasurementId::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->MeasurementId()) : gcnew String("");     
}

void MdfHeader::MeasurementId::set(String^ uuid) {
  if (header_ != nullptr) {
    header_->MeasurementId(String::IsNullOrEmpty(uuid) ?
      std::string() : marshal_as<std::string>(uuid));
  }     
}

String^ MdfHeader::RecorderId::get() {
  return header_ != nullptr ? MdfLibrary::Utf8Conversion(header_->RecorderId()) : gcnew String("");     
}

void MdfHeader::RecorderId::set(String^ uuid) {
  if (header_ != nullptr) {
    header_->RecorderId(String::IsNullOrEmpty(uuid) ?
      std::string() : marshal_as<std::string>(uuid));
  }     
}

int64_t MdfHeader::RecorderIndex::get() {
  return header_ != nullptr ? header_->RecorderIndex() : 0;
}

void MdfHeader::RecorderIndex::set(int64_t index) {
  if (header_ != nullptr) {
    header_->RecorderIndex(index);
  }  
}

uint64_t MdfHeader::StartTime::get() {
  return header_ != nullptr ? header_->StartTime() : 0;
}

void MdfHeader::StartTime::set(uint64_t time) {
  if (header_ != nullptr) {
    header_->StartTime(time);
  }  
}

double MdfHeader::StartAngle::get() {
  return header_ != nullptr  && header_->StartAngle().has_value() ?
    header_->StartAngle().value() : 0;
}

void MdfHeader::StartAngle::set(double angle) {
  if (header_ != nullptr) {
    header_->StartAngle(angle);
  }  
}

double MdfHeader::StartDistance::get() {
  return header_ != nullptr  && header_->StartDistance().has_value() ?
    header_->StartDistance().value() : 0;  
}

void MdfHeader::StartDistance::set(double distance) {
  if (header_ != nullptr) {
    header_->StartDistance(distance);
  }    
}

MdfMetaData^ MdfHeader::MetaData::get() {
  const auto* temp = header_ != nullptr ? header_->MetaData() : nullptr;
  return  temp != nullptr ?
    gcnew MdfMetaData(const_cast<mdf::IMetaData*>(temp)) : nullptr; 
}

array<MdfAttachment^>^ MdfHeader::Attachments::get() {
  if (header_ == nullptr) {
    return gcnew array<MdfAttachment^>(0);
  }
  const auto list = header_->Attachments();
  auto temp = gcnew array<MdfAttachment^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] = gcnew MdfAttachment(list[index]);
  }
  return temp;
}

array<MdfFileHistory^>^ MdfHeader::FileHistories::get() {
  if (header_ == nullptr) {
    return gcnew array<MdfFileHistory^>(0);
  }
  const auto list = header_->FileHistories();
  auto temp = gcnew array<MdfFileHistory^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] = gcnew MdfFileHistory(list[index]);
  }
  return temp;  
}

array<MdfEvent^>^ MdfHeader::Events::get() {
  if (header_ == nullptr) {
    return gcnew array<MdfEvent^>(0);
  }
  const auto list = header_->Events();
  auto temp = gcnew array<MdfEvent^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] = gcnew MdfEvent(list[index]);
  }
  return temp;  
}

array<MdfDataGroup^>^ MdfHeader::DataGroups::get() {
  if (header_ == nullptr) {
    return gcnew array<MdfDataGroup^>(0);
  }
  const auto list = header_->DataGroups();
  auto temp = gcnew array<MdfDataGroup^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] = gcnew MdfDataGroup(list[index]);
  }
  return temp;    
}

MdfAttachment^ MdfHeader::CreateAttachment() {
  auto* temp = header_ != nullptr ?
    header_->CreateAttachment() : nullptr;
  return gcnew MdfAttachment(temp);
}

MdfFileHistory^ MdfHeader::CreateFileHistory() {
  auto* temp = header_ != nullptr ?
    header_->CreateFileHistory() : nullptr;
  return gcnew MdfFileHistory(temp);  
}

#undef CreateEvent
MdfEvent ^ MdfHeader::CreateEvent() {
  auto* temp = header_ != nullptr ?
    header_->CreateEvent() : nullptr;
  return gcnew MdfEvent(temp);    
}

MdfDataGroup^ MdfHeader::CreateDataGroup() {
  auto* temp = header_ != nullptr ?
    header_->CreateDataGroup() : nullptr;
  return gcnew MdfDataGroup(temp);   
}

MdfDataGroup^ MdfHeader::LastDataGroup::get() {
  auto* temp = header_ != nullptr ?
    header_->LastDataGroup() : nullptr;
  return temp != nullptr ? gcnew MdfDataGroup(temp) : nullptr;
}

bool MdfHeader::IsStartAngleUsed::get() {
  return header_ != nullptr ? header_->StartAngle().has_value() : false;
}

bool MdfHeader::IsStartDistanceUsed::get() {
  return header_ != nullptr ? header_->StartDistance().has_value() : false;
}

}
