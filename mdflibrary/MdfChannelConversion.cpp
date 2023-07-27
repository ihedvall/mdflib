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
  if (conversion_ != nullptr) {
    conversion_->Name(MdfLibrary::Utf8Conversion(name));
  }
}

String^ MdfChannelConversion::Description::get() {
  return conversion_ != nullptr ? MdfLibrary::Utf8Conversion(conversion_->Description()) : gcnew String("");  
}
void MdfChannelConversion::Description::set(String^ desc) {
  if (conversion_ != nullptr) {
    conversion_->Description(MdfLibrary::Utf8Conversion(desc));
  }  
}

bool MdfChannelConversion::UnitUsed::get() {
  return conversion_ != nullptr ? conversion_->IsUnitValid() : false;
}

String^ MdfChannelConversion::Unit::get() {
  return conversion_ != nullptr ? MdfLibrary::Utf8Conversion(conversion_->Unit()) : gcnew String(""); 
}

void MdfChannelConversion::Unit::set(String^ unit) {
  if (conversion_ != nullptr) {
    conversion_->Unit(MdfLibrary::Utf8Conversion(unit));
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

Tuple<double, double>^ MdfChannelConversion::Range::get() {
  double min = 0, max = 0;
  if (conversion_ != nullptr) {
    const auto optional = conversion_->Range();
    if (optional.has_value()) {
      min = optional.value().first;
      max = optional.value().second;
    }
  }
  return gcnew Tuple<double, double>(min, max);
}

void MdfChannelConversion::Range::set(Tuple<double, double> ^ range) {
  if (conversion_ != nullptr) {
    conversion_->Range(range->Item1, range->Item2);
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
