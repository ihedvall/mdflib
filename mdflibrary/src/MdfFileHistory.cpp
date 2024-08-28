/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfFileHistory.h"
#include "mdflibrary.h"

namespace MdfLibrary {

int64_t MdfFileHistory::Index::get() {
  return history_ != nullptr ? history_->Index() : 0; 
}

uint64_t MdfFileHistory::Time::get() {
  return history_ != nullptr ? history_->Time() : 0;
}

void MdfFileHistory::Time::set(uint64_t time) {
  if (history_ != nullptr) {
    history_->Time(time);
  }
}

MdfMetaData^ MdfFileHistory::MetaData::get() {
  const auto* temp = history_ != nullptr ? history_->MetaData() : nullptr;
  return temp != nullptr ?
    gcnew MdfMetaData(const_cast<mdf::IMetaData*>(temp)) : nullptr;
}

String^ MdfFileHistory::Description::get() {
  return history_ != nullptr ? MdfLibrary::Utf8Conversion(history_->Description()) : gcnew String("");
}

void MdfFileHistory::Description::set(String^ desc) {
  if (history_ != nullptr) {
    history_->Description(MdfLibrary::Utf8Conversion(desc));
  }    
}

String^ MdfFileHistory::ToolName::get() {
  return history_ != nullptr ? MdfLibrary::Utf8Conversion(history_->ToolName()) : gcnew String("");  
}

void MdfFileHistory::ToolName::set(String^ name) {
if (history_ != nullptr) {
    history_->ToolName(MdfLibrary::Utf8Conversion(name));
  }
}

String^ MdfFileHistory::ToolVendor::get() {
  const auto temp = history_ != nullptr ?
    history_->ToolVendor() : std::string();
  return gcnew String( temp.c_str()); 
}

void MdfFileHistory::ToolVendor::set(String^ vendor) {
if (history_ != nullptr) {
    history_->ToolVendor(MdfLibrary::Utf8Conversion(vendor));
  }  
}

String^ MdfFileHistory::ToolVersion::get() {
  const auto temp = history_ != nullptr ?
    history_->ToolVersion() : std::string();
  return gcnew String( temp.c_str());   
}

void MdfFileHistory::ToolVersion::set(String^ version) {
if (history_ != nullptr) {
    history_->ToolVersion(MdfLibrary::Utf8Conversion(version));
  }    
}

String^ MdfFileHistory::UserName::get() {
  const auto temp = history_ != nullptr ?
    history_->UserName() : std::string();
  return gcnew String( temp.c_str());
}

void MdfFileHistory::UserName::set(String^ user) {
  if (history_ != nullptr) {
    history_->UserName(MdfLibrary::Utf8Conversion(user));
  }      
}

void MdfFileHistory::SetStartTimeLocal(const uint64_t time) {
  if (history_ != nullptr) {
    history_->SetStartTimeLocal(time);
  }
}

void MdfFileHistory::SetStartTimeWithZone(const uint64_t time, const int16_t tz_offset_min,
                                          const int16_t dst_offset_min) {
  if (history_ != nullptr) {
    history_->SetStartTimeWithZone(time, tz_offset_min, dst_offset_min);
  }
}

void MdfFileHistory::SetStartTimeUtc(const uint64_t time) {
  if (history_ != nullptr) {
    history_->SetStartTimeUtc(time);
  }
}

MdfFileHistory::MdfFileHistory(mdf::IFileHistory* history)
  : history_(history) {
}

}
