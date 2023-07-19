/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfChannelConversion.h"
#include "mdflibrary.h"

using namespace msclr::interop;

namespace MdfLibrary {

Int64 MdfChannelConversion::Index::get() {
  return conversion_ != nullptr ? conversion_->Index() : 0;
}

String^ MdfChannelConversion::Name::get() {
  return conversion_ != nullptr ? MdfLibrary::Utf8Conversion(conversion_->Name()) : gcnew String("");
}

void MdfChannelConversion::Name::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
  std::string() : marshal_as<std::string>(name);
  if (conversion_ != nullptr) {
    conversion_->Name(temp);
  }
}

String^ MdfChannelConversion::Description::get() {
  return conversion_ != nullptr ? MdfLibrary::Utf8Conversion(conversion_->Description()) : gcnew String("");  
}
void MdfChannelConversion::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
  std::string() : marshal_as<std::string>(desc);
  if (conversion_ != nullptr) {
    conversion_->Description(temp);
  }  
}

bool MdfChannelConversion::UnitUsed::get() {
  return conversion_ != nullptr ? conversion_->IsUnitValid() : false;
}

String^ MdfChannelConversion::Unit::get() {
  return conversion_ != nullptr ? MdfLibrary::Utf8Conversion(conversion_->Unit()) : gcnew String(""); 
}

void MdfChannelConversion::Unit::set(String^ unit) {
  const auto temp = String::IsNullOrEmpty(unit) ?
  std::string() : marshal_as<std::string>(unit);
  if (conversion_ != nullptr) {
    conversion_->Unit(temp);
  }  
}

ConversionType MdfChannelConversion::Type::get() {
  const auto temp = conversion_ != nullptr ?
    conversion_->Type() : mdf::ConversionType::NoConversion; 
  return static_cast<ConversionType>(temp);
}

void MdfChannelConversion::Type::set(ConversionType type) {
  const auto temp = static_cast<mdf::ConversionType> (type);
  if (conversion_ != nullptr) {
    conversion_->Type(temp);
  }
}

bool MdfChannelConversion::PrecisionUsed::get() {
  return conversion_ != nullptr ? conversion_->IsDecimalUsed() : false;
}

unsigned char MdfChannelConversion::Precision::get() {
  return conversion_ != nullptr ? conversion_->Decimals() : 2;   
}

void MdfChannelConversion::Precision::set(unsigned char precision) {
  if (conversion_ != nullptr) {
    conversion_->Decimals(precision);
  }
}

bool MdfChannelConversion::RangeUsed::get() {
  bool used = false;
  if (conversion_ != nullptr) {
    const auto optional = conversion_->Range();
    used = optional.has_value();
  }  
  return used;
}

double MdfChannelConversion::RangeMin::get() {
  double min = 0;
  if (conversion_ != nullptr) {
      const auto optional = conversion_->Range();
    if (optional.has_value()) {
      min = optional.value().first;
    }
  }
  return min;
}

void MdfChannelConversion::RangeMin::set(double min) {
  if (conversion_ != nullptr) {
    const auto optional = conversion_->Range();
    double max = 0;
    if (optional.has_value()) {
      max = optional.value().second;
    }
    conversion_->Range(min,max);
  }
}

double MdfChannelConversion::RangeMax::get() {
  double max = 0;
  if (conversion_ != nullptr) {
    const auto optional = conversion_->Range();
    if (optional.has_value()) {
      max = optional.value().first;
    }
  }
  return max;  
}

void MdfChannelConversion::RangeMax::set(double max) {
  if (conversion_ != nullptr) {
    const auto optional = conversion_->Range();
    double min = 0;
    if (optional.has_value()) {
      min = optional.value().second;
    }
    conversion_->Range(min,max);
  }
}

unsigned short MdfChannelConversion::Flags::get() {
  return conversion_ != nullptr ? conversion_->Flags() : 0;
}

void MdfChannelConversion::Flags::set(unsigned short flags) {
  if (conversion_ != nullptr) {
    conversion_->Flags(flags);
  }  
}

MdfChannelConversion^ MdfChannelConversion::Inverse::get() {
  auto* temp = conversion_ != nullptr ?
    const_cast<mdf::IChannelConversion*>(conversion_->Inverse()) : nullptr;
  return temp != nullptr ? gcnew MdfChannelConversion(temp) : nullptr;
}

MdfChannelConversion^ MdfChannelConversion::CreateInverse() {
  auto* temp = conversion_ != nullptr ?
    conversion_->CreateInverse() : nullptr;
  return gcnew MdfChannelConversion(temp);  
}

MdfChannelConversion::
MdfChannelConversion(mdf::IChannelConversion* conversion) {
  conversion_ = conversion;
}

}
