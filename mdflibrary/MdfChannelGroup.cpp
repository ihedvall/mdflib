/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfChannelGroup.h"
using namespace msclr::interop;

namespace MdfLibrary {
int64_t MdfChannelGroup::Index::get() {
  return group_ != nullptr ? group_->Index() : 0; 
}

uint64_t MdfChannelGroup::RecordId::get() {
  return group_ != nullptr ? group_->RecordId() : 0; 
}

void MdfChannelGroup::RecordId::set(uint64_t record_id) {
  if (group_ != nullptr) {
    group_->RecordId(record_id);
  }    
}

String^ MdfChannelGroup::Name::get() {
  const auto temp = group_ != nullptr ?
    group_->Name() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfChannelGroup::Name::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
  std::string() : marshal_as<std::string>(name);
  if (group_ != nullptr) {
    group_->Name(temp);
  }    
}

String^ MdfChannelGroup::Description::get() {
  const auto temp = group_ != nullptr ?
    group_->Description() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfChannelGroup::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
  std::string() : marshal_as<std::string>(desc);
  if (group_ != nullptr) {
    group_->Description(temp);
  }    
}

uint64_t MdfChannelGroup::NofSamples::get() {
  return group_ != nullptr ? group_->NofSamples() : 0; 
}

void MdfChannelGroup::NofSamples::set(uint64_t samples) {
  if (group_ != nullptr) {
    group_->NofSamples(samples);
  }    
}

uint16_t MdfChannelGroup::Flags::get() {
  return group_ != nullptr ? group_->Flags() : 0; 
}

void MdfChannelGroup::Flags::set(uint16_t flags) {
  if (group_ != nullptr) {
    group_->Flags(flags);
  }    
}

wchar_t MdfChannelGroup::PathSeparator::get() {
  return group_ != nullptr ? group_->PathSeparator() : L'/';
}

void MdfChannelGroup::PathSeparator::set(wchar_t sep) {
  if (group_ != nullptr) {
    group_->PathSeparator(sep);
  }
}

array<MdfChannel^>^ MdfChannelGroup::Channels::get() {
  array<MdfChannel^>^ temp;
  if (group_ != nullptr) {
    const auto list = group_->Channels();
    temp = gcnew array<MdfChannel^>(static_cast<int>(list.size()));
    for (size_t index = 0; index < list.size(); ++index) {
      temp[static_cast<int>(index)] = gcnew MdfChannel(list[index]);
    }  
  } else {
    temp = gcnew array<MdfChannel^>(0);
  }
  return temp;
}

MdfSourceInformation^ MdfChannelGroup::SourceInformation::get() {
  auto* temp = group_ != nullptr ?
    const_cast<mdf::ISourceInformation*>(group_->SourceInformation()) : nullptr;
  return temp != nullptr ? gcnew MdfSourceInformation(temp) : nullptr;
}

MdfChannel^ MdfChannelGroup::GetXChannel(const MdfChannel^ ref_channel) {
  const auto* ref = ref_channel != nullptr ?
      ref_channel->channel_ : nullptr;
  mdf::IChannel *temp = nullptr;
  if (group_ != nullptr && ref != nullptr) {
    temp = const_cast<mdf::IChannel*>(group_->GetXChannel(*ref));
  }
  return temp != nullptr ? gcnew MdfChannel(temp) : nullptr;
}

MdfChannelGroup::MdfChannelGroup(mdf::IChannelGroup* group)
  : group_(group) {
  
}

}
