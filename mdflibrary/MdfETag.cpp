/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>
#include "MdfETag.h"

using namespace msclr::interop;

namespace MdfLibrary {

MdfETag::MdfETag()
  : tag_(new mdf::ETag) {
}

MdfETag::MdfETag(MdfETag^& tag)
  : tag_(new mdf::ETag) {
  *tag_ = *tag->tag_;
}

MdfETag::~MdfETag() {
  this->!MdfETag();
}

String^ MdfETag::Name::get() {
  const auto temp = tag_ != nullptr ?
    tag_->Name() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);  
}

void MdfETag::Name::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
    std::string() : marshal_as<std::string>(name);
  if (tag_ != nullptr) {
    tag_->Name(temp);
 }   
}

String^ MdfETag::Description::get() {
  const auto temp = tag_ != nullptr ?
    tag_->Description() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);    
}

void MdfETag::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
    std::string() : marshal_as<std::string>(desc);
  if (tag_ != nullptr) {
    tag_->Description(temp);
  }    
}

String^ MdfETag::Unit::get() {
  const auto temp = tag_ != nullptr ?
    tag_->Unit() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);     
}

void MdfETag::Unit::set(String^ unit) {
  const auto temp = String::IsNullOrEmpty(unit) ?
    std::string() : marshal_as<std::string>(unit);
  if (tag_ != nullptr) {
    tag_->Unit(temp);
  }      
}

String^ MdfETag::UnitRef::get() {
  const auto temp = tag_ != nullptr ?
    tag_->UnitRef() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);    
}

void MdfETag::UnitRef::set(String^ unit) {
  const auto temp = String::IsNullOrEmpty(unit) ?
    std::string() : marshal_as<std::string>(unit);
  if (tag_ != nullptr) {
    tag_->UnitRef(temp);
  }        
}

String^ MdfETag::Type::get() {
  const auto temp = tag_ != nullptr ?
    tag_->Type() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);   
}

void MdfETag::Type::set(String^ type) {
  const auto temp = String::IsNullOrEmpty(type) ?
    std::string() : marshal_as<std::string>(type);
  if (tag_ != nullptr) {
    tag_->Type(temp);
  }    
}

ETagDataType MdfETag::DataType::get() {
  return tag_ != nullptr ?
    static_cast<ETagDataType>(tag_->DataType()) : ETagDataType::StringType; 
}

void MdfETag::DataType::set(ETagDataType type) {
  if (tag_ != nullptr) {
    tag_->DataType(static_cast<mdf::ETagDataType>( tag_->DataType()));
  }  
}

String^ MdfETag::Language::get() {
  const auto temp = tag_ != nullptr ?
    tag_->Language() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);     
}

void MdfETag::Language::set(String^ language) {
  const auto temp = String::IsNullOrEmpty(language) ?
    std::string() : marshal_as<std::string>(language);
  if (tag_ != nullptr) {
    tag_->Language(temp);
  }    
}

bool MdfETag::ReadOnly::get() {
  return tag_ != nullptr ? tag_->ReadOnly() : false;
}

void MdfETag::ReadOnly::set(bool read_only) {
  if (tag_ != nullptr) {
    tag_->ReadOnly(read_only);
  }
}

String^ MdfETag::ValueAsString::get() {
  const auto temp = tag_ != nullptr ?
    tag_->Value<std::string>() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);    
}

void MdfETag::ValueAsString::set(String^ value) {
  const auto temp = String::IsNullOrEmpty(value) ?
    std::string() : marshal_as<std::string>(value);
   if (tag_ != nullptr) {
     tag_->Value(temp);
   } 
}

double MdfETag::ValueAsFloat::get() {
  return tag_ != nullptr ? tag_->Value<double>() : 0.0;
}

void MdfETag::ValueAsFloat::set(double value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

bool MdfETag::ValueAsBoolean::get() {
  return tag_ != nullptr ? tag_->Value<bool>() : false;
}

void MdfETag::ValueAsBoolean::set(bool value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

int64_t MdfETag::ValueAsSigned::get() {
  return tag_ != nullptr ? tag_->Value<int64_t>() : 0;
}

void MdfETag::ValueAsSigned::set(int64_t value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

uint64_t MdfETag::ValueAsUnsigned::get() {
  return tag_ != nullptr ? tag_->Value<uint64_t>() : 0;
}

void MdfETag::ValueAsUnsigned::set(uint64_t value) {
  if (tag_ != nullptr) {
    tag_->Value(value);
  }
}

void MdfETag::!MdfETag() {
  delete tag_;
  tag_ = nullptr;
}

MdfETag::MdfETag(const mdf::ETag& tag)
: tag_( new mdf::ETag )
{
  *tag_ = tag;
}

}
