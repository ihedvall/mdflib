/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <string>
#include <msclr/marshal_cppstd.h>
#include "MdfMetaData.h"

using namespace msclr::interop;

namespace MdfLibrary {

String^ MdfMetaData::PropertyAsString::get(String^ index) {
  const auto key = String::IsNullOrEmpty(index) ?
    std::string() : marshal_as<std::string>(index);
  const auto temp = meta_data_ != nullptr ?
   meta_data_->StringProperty(key) : std::string();
  return gcnew String(temp.c_str());   
}

void MdfMetaData::PropertyAsString::set(String^ index, String^ prop) {
  if (String::IsNullOrEmpty(index)) {
    return;
  }
  const auto key = marshal_as<std::string>(index);
  const auto value = String::IsNullOrEmpty(prop) ?
    std::string() : marshal_as<std::string>(prop);
  if (meta_data_ != nullptr) {
    meta_data_->StringProperty(key, value);
  }
}

double MdfMetaData::PropertyAsFloat::get(String^ index) {
  const auto key = String::IsNullOrEmpty(index) ?
    std::string() : marshal_as<std::string>(index);
  return meta_data_ != nullptr ? meta_data_->FloatProperty(key) : 0.0;
}

void MdfMetaData::PropertyAsFloat::set(String^ index, double prop) {
  if (String::IsNullOrEmpty(index)) {
    return;
  }
  const auto key = marshal_as<std::string>(index);
  if (meta_data_ != nullptr) {
    meta_data_->FloatProperty(key, prop);
  }  
}

array<MdfETag^>^ MdfMetaData::CommonProperties::get() {
  if (meta_data_ == nullptr) {
    return gcnew array<MdfETag^>(0);    
  }
  const auto list = meta_data_->CommonProperties();
  array<MdfETag^>^ temp_list =
    gcnew array<MdfETag^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    const auto& temp = list[index];
    temp_list[static_cast<int>(index)] = gcnew MdfETag(temp);
  }
  return temp_list;
}

void MdfMetaData::CommonProperties::set(array<MdfETag^>^ prop_list) {
  std::vector<mdf::ETag> temp_list;
  for (int index = 0; index < prop_list->Length; ++index) {
    const auto* tag = prop_list[index]->tag_;
    temp_list.emplace_back(tag != nullptr ? *tag : mdf::ETag());
  }
  if (meta_data_ != nullptr) {
    meta_data_->CommonProperties(temp_list);
  }
}

String^ MdfMetaData::XmlSnippet::get() {
  const auto temp = meta_data_ != nullptr ?
   meta_data_->XmlSnippet() : std::string();
  return gcnew String(temp.c_str());   
}

void MdfMetaData::XmlSnippet::set(String^ xml) {
  const auto temp = String::IsNullOrEmpty(xml) ?
    std::string() : marshal_as<std::string>(xml);
  if (meta_data_ != nullptr) {
    meta_data_->XmlSnippet(temp);
  }   
}

MdfETag^ MdfMetaData::GetCommonProperty(String^ name) {
  const auto key = String::IsNullOrEmpty(name) ?
    std::string() : marshal_as<std::string>(name);
  const auto temp = meta_data_ != nullptr ?
    meta_data_->CommonProperty(key) : mdf::ETag();
  return gcnew MdfETag(temp);
}

void MdfMetaData::AddCommonProperty(MdfETag^ tag) {
  if (meta_data_ != nullptr && tag->tag_ != nullptr) {
    meta_data_->CommonProperty(*tag->tag_);
  }
}


MdfMetaData::MdfMetaData(mdf::IMetaData* meta_data)
  : meta_data_(meta_data) {
  
}

}
