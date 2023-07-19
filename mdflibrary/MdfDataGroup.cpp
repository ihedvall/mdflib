/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>
#include "MdfDataGroup.h"
using namespace msclr::interop;

namespace MdfLibrary {

int64_t MdfDataGroup::Index::get() {
  return group_ != nullptr ? group_->Index() : 0; 
}

String^ MdfDataGroup::Description::get() {
  const auto temp = group_ != nullptr ?
  group_->Description() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfDataGroup::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
    std::string() : marshal_as<std::string>(desc);
  if (group_ != nullptr) {
    group_->Description(temp);
  }    
}

uint8_t MdfDataGroup::RecordIdSize::get() {
  return group_ != nullptr ? group_->RecordIdSize() : 0;
}

void MdfDataGroup::RecordIdSize::set(uint8_t id_size) {
  if (group_ != nullptr) {
    group_->RecordIdSize(id_size);
  }
}

array<MdfChannelGroup^>^ MdfDataGroup::ChannelGroups::get() {
  if (group_ == nullptr) {
    return gcnew array<MdfChannelGroup^>(0);
  }
  const auto list = group_->ChannelGroups();
  auto temp = gcnew array<MdfChannelGroup^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] = gcnew MdfChannelGroup(list[index]);
  }
  return temp;
}

MdfMetaData^ MdfDataGroup::MetaData::get() {
  const auto* temp = group_ != nullptr ?
    group_->MetaData() : nullptr;
  return temp != nullptr ?
    gcnew MdfMetaData(const_cast<mdf::IMetaData*>(temp)) : nullptr; 
}

MdfChannelGroup^ MdfDataGroup::CreateChannelGroup() {
  auto* temp = group_ != nullptr ?
    group_->CreateChannelGroup() : nullptr;
  return gcnew MdfChannelGroup(temp);
}

MdfMetaData^ MdfDataGroup::CreateMetaData() {
  auto* temp = group_ != nullptr ?
    group_->CreateMetaData() : nullptr;
  return gcnew MdfMetaData(temp);  
}

bool MdfDataGroup::IsRead::get() {
  return group_ != nullptr ? group_->IsRead() : false;
}

MdfChannelGroup^ MdfDataGroup::FindParentChannelGroup(MdfChannel^ channel) {
  const auto* input =
    channel != nullptr ? channel->channel_ : nullptr;
  if (input == nullptr) {
    return nullptr;
  }
  
  const auto* temp = group_ != nullptr ?
    group_->FindParentChannelGroup(*input) : nullptr;
  return temp != nullptr ?
    gcnew MdfChannelGroup(const_cast<mdf::IChannelGroup*>(temp)) : nullptr;  
}

void MdfDataGroup::ResetSample() {
  if (group_ != nullptr) {
    group_->ResetSample();
  }
}

MdfDataGroup::MdfDataGroup(mdf::IDataGroup* group)
  : group_(group) {

}

}
