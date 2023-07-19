/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfFileHistory.h"

using namespace msclr::interop;

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
  const auto temp = history_ != nullptr ?
    history_->Description() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfFileHistory::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
    std::string() : marshal_as<std::string>(desc);
  if (history_ != nullptr) {
    history_->Description(temp);
  }    
}

String^ MdfFileHistory::ToolName::get() {
  const auto temp = history_ != nullptr ?
    history_->ToolName() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);  
}

void MdfFileHistory::ToolName::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
    std::string() : marshal_as<std::string>(name);
  if (history_ != nullptr) {
    history_->ToolName(temp);
  }
}

String^ MdfFileHistory::ToolVendor::get() {
  const auto temp = history_ != nullptr ?
    history_->ToolVendor() : std::string();
  return gcnew String( temp.c_str()); 
}

void MdfFileHistory::ToolVendor::set(String^ vendor) {
  const auto temp = String::IsNullOrEmpty(vendor) ?
    std::string() : marshal_as<std::string>(vendor);
  if (history_ != nullptr) {
    history_->ToolVendor(temp);
  }  
}

String^ MdfFileHistory::ToolVersion::get() {
  const auto temp = history_ != nullptr ?
    history_->ToolVersion() : std::string();
  return gcnew String( temp.c_str());   
}

void MdfFileHistory::ToolVersion::set(String^ version) {
  const auto temp = String::IsNullOrEmpty(version) ?
    std::string() : marshal_as<std::string>(version);
  if (history_ != nullptr) {
    history_->ToolVersion(temp);
  }    
}

String^ MdfFileHistory::UserName::get() {
  const auto temp = history_ != nullptr ?
    history_->UserName() : std::string();
  return gcnew String( temp.c_str());
}

void MdfFileHistory::UserName::set(String^ user) {
  const auto temp = String::IsNullOrEmpty(user) ?
    std::string() : marshal_as<std::string>(user);
  if (history_ != nullptr) {
    history_->UserName(marshal_as<std::string>(user));
  }      
}

MdfFileHistory::MdfFileHistory(mdf::IFileHistory* history)
  : history_(history) {
}

}
