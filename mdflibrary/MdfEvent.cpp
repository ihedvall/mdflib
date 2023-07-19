/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfEvent.h"
using namespace msclr::interop;

namespace MdfLibrary {

int64_t MdfEvent::Index::get() {
  return event_ != nullptr ? event_->Index() : 0; 
}

String^ MdfEvent::Name::get() {
  const auto temp = event_ != nullptr ?
    event_->Name() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfEvent::Name::set(String^ name) {
  const auto temp = String::IsNullOrEmpty(name) ?
    std::string() : marshal_as<std::string>(name);
  if (event_ != nullptr) {
    event_->Name(temp);
  }    
}

String^ MdfEvent::Description::get() {
  const auto temp = event_ != nullptr ?
    event_->Description() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfEvent::Description::set(String^ desc) {
  const auto temp = String::IsNullOrEmpty(desc) ?
    std::string() : marshal_as<std::string>(desc);
  if (event_ != nullptr) {
    event_->Description(temp);
  }    
}

String^ MdfEvent::Group::get() {
  const auto temp = event_ != nullptr ?
    event_->GroupName() : std::string();

  array<unsigned char> ^ c_array =
      gcnew array<unsigned char>(temp.length());

  for (int i = 0; i < temp.length(); i++)
    c_array[i] = temp[i];

  System::Text::Encoding ^ u8enc = System::Text::Encoding::UTF8;

  return u8enc->GetString(c_array);
}

void MdfEvent::Group::set(String^ group) {
  const auto temp = String::IsNullOrEmpty(group) ?
    std::string() : marshal_as<std::string>(group);
  if (event_ != nullptr) {
    event_->GroupName(temp);
  }    
}

EventType MdfEvent::Type::get() {
  return event_ != nullptr ?
    static_cast<EventType>(event_->Type()) : EventType::Trigger;
}

void MdfEvent::Type::set(EventType type) {
  if (event_ != nullptr) {
    event_->Type(static_cast<mdf::EventType>(type));
  }
}

SyncType MdfEvent::Sync::get() {
  return event_ != nullptr ?
    static_cast<SyncType>(event_->Sync()) : SyncType::SyncTime;
}

void MdfEvent::Sync::set(SyncType type) {
  if (event_ != nullptr) {
    event_->Sync(static_cast<mdf::SyncType>(type));
  }
}

RangeType MdfEvent::Range::get() {
  return event_ != nullptr ?
    static_cast<RangeType>(event_->Range()) : RangeType::RangePoint;
}

void MdfEvent::Range::set(RangeType type) {
  if (event_ != nullptr) {
    event_->Range(static_cast<mdf::RangeType>(type));
  }
}

EventCause MdfEvent::Cause::get() {
  return event_ != nullptr ?
    static_cast<EventCause>(event_->Cause()) : EventCause::CauseOther;
}

void MdfEvent::Cause::set(EventCause cause) {
  if (event_ != nullptr) {
    event_->Cause(static_cast<mdf::EventCause>(cause));
  }
}

size_t MdfEvent::CreatorIndex::get() {
  return event_ != nullptr ? event_->CreatorIndex() : 0;
}

void MdfEvent::CreatorIndex::set(size_t index) {
  if (event_ != nullptr) {
    event_->CreatorIndex(index);    
  }
}

int64_t MdfEvent::SyncValue::get() {
  return event_ != nullptr ? event_->SyncValue() : 0;
}

void MdfEvent::SyncValue::set(int64_t value) {
  if (event_ != nullptr) {
    event_->SyncValue(value);    
  }
}

double MdfEvent::SyncFactor::get() {
  return event_ != nullptr ? event_->SyncFactor() : 0;
}

void MdfEvent::SyncFactor::set(double factor) {
  if (event_ != nullptr) {
    event_->SyncFactor(factor);    
  }
}

MdfEvent^ MdfEvent::ParentEvent::get() {
  auto* temp = event_ != nullptr && event_->ParentEvent() != nullptr?
    const_cast<mdf::IEvent*>(event_->ParentEvent()) : nullptr;
  return temp != nullptr ? gcnew MdfEvent(temp) : nullptr;
}

void MdfEvent::ParentEvent::set(MdfEvent^ parent) {
  if (event_ != nullptr) {
    event_->ParentEvent(parent != nullptr ? parent->event_ : nullptr);
  } 
}

MdfEvent^ MdfEvent::RangeEvent::get() {
  auto* temp = event_ != nullptr && event_->RangeEvent() != nullptr?
    const_cast<mdf::IEvent*>(event_->RangeEvent()) : nullptr;
  return temp != nullptr ? gcnew MdfEvent(temp) : nullptr;
}

void MdfEvent::RangeEvent::set(MdfEvent^ event) {
  if (event_ != nullptr) {
    event_->RangeEvent(event != nullptr ? event->event_ : nullptr);
  } 
}

array<MdfAttachment^>^ MdfEvent::Attachments::get() {
  if (event_ == nullptr) {
    return gcnew array<MdfAttachment^>(0);
  }
  const auto& list = event_->Attachments();
  auto temp =
    gcnew array<MdfAttachment^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] =
      gcnew MdfAttachment(const_cast<mdf::IAttachment*>(list[index])); 
  }
  return temp;
}

double MdfEvent::PreTrig::get() {
  return event_ != nullptr ? event_->PreTrig() : 0.0;
}

void MdfEvent::PreTrig::set(double time) {
  if (event_ != nullptr) {
    event_->PreTrig(time);
  }
}

double MdfEvent::PostTrig::get() {
  return event_ != nullptr ? event_->PostTrig() : 0.0;
}

void MdfEvent::PostTrig::set(double time) {
  if (event_ != nullptr) {
    event_->PostTrig(time);
  }
}

MdfMetaData^ MdfEvent::MetaData::get() {
  auto* temp = event_ != nullptr && event_->MetaData() != nullptr?
   const_cast<mdf::IMetaData*>(event_->MetaData()) : nullptr;
  return temp != nullptr ? gcnew MdfMetaData(temp) : nullptr; 
}

void MdfEvent::AddAttachment(MdfAttachment^ attachment) {
  if (event_ != nullptr && attachment != nullptr) {
    event_->AddAttachment(attachment->attachment_);
  }
}

MdfEvent::MdfEvent(mdf::IEvent* event)
  : event_(event) {
  
}


}
