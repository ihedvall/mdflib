/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfChannel.h"
#include "mdflibrary.h"

using namespace msclr::interop;

namespace MdfLibrary {

int64_t MdfChannel::Index::get() {
  return channel_ != nullptr ? channel_->Index() : 0;
}

String^ MdfChannel::Name::get() {
  return channel_ != nullptr ? MdfLibrary::Utf8Conversion(channel_->Name()) : gcnew String("");
}

void MdfChannel::Name::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
    std::string() : marshal_as<std::string>(name);
  if (channel_ != nullptr) {
    channel_->Name(temp);
  }
}

String^ MdfChannel::DisplayName::get() {
  return channel_ != nullptr ? MdfLibrary::Utf8Conversion(channel_->DisplayName()) : gcnew String("");
}

void MdfChannel::DisplayName::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
  std::string() : marshal_as<std::string>(name);
  if (channel_ != nullptr) {
    channel_->DisplayName(temp);
  }  
}

String^ MdfChannel::Description::get() {
  return channel_ != nullptr ? MdfLibrary::Utf8Conversion(channel_->Description()) : gcnew String("");   
}

void MdfChannel::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
  std::string() : marshal_as<std::string>(desc);
   if (channel_ != nullptr) {
     channel_->Description(temp);
   }   
}

bool MdfChannel::UnitUsed::get() {
  return channel_ != nullptr ? channel_->IsUnitValid() : false;
}

String^ MdfChannel::Unit::get() {
  return channel_ != nullptr ? MdfLibrary::Utf8Conversion(channel_->Unit()) : gcnew String("");
}

void MdfChannel::Unit::set(String^ unit) {
  const auto temp = String::IsNullOrEmpty(unit) ?
    std::string() : marshal_as<std::string>(unit);
  if (channel_ != nullptr) {
     channel_->Unit(temp);
   }   
}

ChannelType MdfChannel::Type::get() {
  const auto temp = channel_ != nullptr ?
    channel_->Type() : mdf::ChannelType::FixedLength;
  return static_cast<ChannelType>(temp);
}

void MdfChannel::Type::set(ChannelType type) {
  if (channel_ != nullptr) {
    channel_->Type(static_cast<mdf::ChannelType>(type));
  }
}

ChannelSyncType MdfChannel::Sync::get() {
  const auto temp = channel_ != nullptr ?
    channel_->Sync() : mdf::ChannelSyncType::Time;
  return static_cast<ChannelSyncType>(temp);  
}

void MdfChannel::Sync::set(ChannelSyncType type) {
  if (channel_ != nullptr) {
    channel_->Sync(static_cast<mdf::ChannelSyncType>(type));
  }  
}

ChannelDataType MdfChannel::DataType::get() {
  const auto temp = channel_ != nullptr ?
    channel_->DataType() : mdf::ChannelDataType::FloatLe;
  return static_cast<ChannelDataType>(temp);   
}

void MdfChannel::DataType::set(ChannelDataType type) {
  if (channel_ != nullptr) {
    channel_->DataType(static_cast<mdf::ChannelDataType>(type));
  }    
}

size_t MdfChannel::DataBytes::get() {
  return channel_ != nullptr ? channel_->DataBytes() : 0;
}

void MdfChannel::DataBytes::set(size_t bytes) {
  if (channel_ != nullptr) {
    channel_->DataBytes(bytes);
  }
}

bool MdfChannel::PrecisionUsed::get() {
  return channel_ != nullptr ? channel_->IsDecimalUsed() : false;
}

unsigned char MdfChannel::Precision::get() {
  return channel_ != nullptr ? channel_->Decimals() : 2;
}

bool MdfChannel::RangeUsed::get() {
  bool used = false;
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    used = optional.has_value();
  }  
  return used;
}

double MdfChannel::RangeMin::get() {
  double min = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    if (optional.has_value()) {
      min = optional.value().first;
    }
  }
  return min;
}

void MdfChannel::RangeMin::set(double min) {
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    double max = 0;
    if (optional.has_value()) {
      max = optional.value().second;
    }
    channel_->Range(min,max);
  }
}

double MdfChannel::RangeMax::get() {
  double max = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    if (optional.has_value()) {
      max = optional.value().first;
    }
  }
  return max;  
}

void MdfChannel::RangeMax::set(double max) {
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    double min = 0;
    if (optional.has_value()) {
      min = optional.value().second;
    }
    channel_->Range(min,max);
  }
}

bool MdfChannel::LimitUsed::get() {
  bool used = false;
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    used = optional.has_value();
  }  
  return used;
}

double MdfChannel::LimitMin::get() {
  double min = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    if (optional.has_value()) {
      min = optional.value().first;
    }
  }
  return min;
}

void MdfChannel::LimitMin::set(double min) {
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    double max = 0;
    if (optional.has_value()) {
      max = optional.value().second;
    }
    channel_->Limit(min,max);
  }
}

double MdfChannel::LimitMax::get() {
  double max = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    if (optional.has_value()) {
      max = optional.value().first;
    }
  }
  return max;  
}

void MdfChannel::LimitMax::set(double max) {
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    double min = 0;
    if (optional.has_value()) {
      min = optional.value().second;
    }
    channel_->Limit(min,max);
  }
}

bool MdfChannel::ExtLimitUsed::get() {
  bool used = false;
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    used = optional.has_value();
  }  
  return used;
}

double MdfChannel::ExtLimitMin::get() {
  double min = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    if (optional.has_value()) {
      min = optional.value().first;
    }
  }
  return min;
}

void MdfChannel::ExtLimitMin::set(double min) {
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    double max = 0;
    if (optional.has_value()) {
      max = optional.value().second;
    }
    channel_->ExtLimit(min,max);
  }
}

double MdfChannel::ExtLimitMax::get() {
  double max = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    if (optional.has_value()) {
      max = optional.value().first;
    }
  }
  return max;  
}

void MdfChannel::ExtLimitMax::set(double max) {
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    double min = 0;
    if (optional.has_value()) {
      min = optional.value().second;
    }
    channel_->ExtLimit(min,max);
  }
}

double MdfChannel::SamplingRate::get() {
  return channel_ != nullptr ? channel_->SamplingRate() : 0;
}

void MdfChannel::SamplingRate::set(double rate) {
  if (channel_ != nullptr) {
    channel_->SamplingRate(rate);
  }
}

MdfChannelConversion^ MdfChannel::ChannelConversion::get() {
  const auto* conversion = channel_ != nullptr ?
    channel_->ChannelConversion() : nullptr;
  return conversion != nullptr ?
    gcnew MdfChannelConversion(const_cast<mdf::IChannelConversion*>(conversion))
    : nullptr;
}

MdfChannel::MdfChannel(mdf::IChannel* channel) {
  channel_ = channel;
}

}
