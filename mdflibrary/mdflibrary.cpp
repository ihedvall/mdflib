/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <string>
#include <mdf/mdffactory.h>
#include <mdf/mdfreader.h>
#include <msclr/marshal_cppstd.h>

#include "mdflibrary.h"

using namespace msclr::interop;

namespace {

void LogFunc(mdf::MdfLogSeverity severity,
  const std::string& function,
  const std::string& message) {
  const auto sev = static_cast<MdfLibrary::MdfLogSeverity>(severity);
  auto func = gcnew String(function.c_str());
  auto text = gcnew String(message.c_str());
  
  MdfLibrary::MdfLibrary::Instance->FireLogEvent(sev, func, text);
}

void NoLog(mdf::MdfLogSeverity severity,
             const std::string& function,
             const std::string& message) {
}

}
namespace MdfLibrary {
MdfLibrary::MdfLibrary() {
  mdf::MdfFactory::SetLogFunction2(LogFunc);  
}

MdfLibrary::!MdfLibrary() {
  mdf::MdfFactory::SetLogFunction2(NoLog);
}

MdfLibrary::~MdfLibrary() {
  this->!MdfLibrary();
}

MdfLibrary^ MdfLibrary::Instance::get() {
  if (instance_ == nullptr) {
    instance_ = gcnew MdfLibrary();
  }
  return instance_;
}

bool MdfLibrary::IsMdfFile(String^ filename) {
  return String::IsNullOrEmpty(filename) ?
    false : mdf::IsMdfFile(marshal_as<std::string>(filename));
}

MdfChannelObserver^ MdfLibrary::CreateChannelObserver(MdfDataGroup^ data_group,
    MdfChannelGroup^ channel_group, MdfChannel^ channel) {
  if (data_group == nullptr || channel_group == nullptr ||
    channel == nullptr) {
    return nullptr;
  }
  if (data_group->group_ == nullptr || channel_group->group_ == nullptr
      || channel->channel_ == nullptr) {
    return nullptr;
  }
  auto observer = mdf::CreateChannelObserver(*data_group->group_,
    *channel_group->group_, *channel->channel_);
  if (!observer) {
    return nullptr;
  }
  auto temp = gcnew MdfChannelObserver(observer.get());
  observer.release();
  return temp;
}

MdfChannelObserver^ MdfLibrary::CreateChannelObserverByChannelName(
    MdfDataGroup^ data_group, String^ channel_name) {
  if (data_group == nullptr || String::IsNullOrEmpty(channel_name)) {
    return nullptr;
  }
  if (data_group->group_ == nullptr) {
    return nullptr;
  }
  auto observer = mdf::CreateChannelObserver(*data_group->group_,
   marshal_as<std::string>(channel_name));
  if (!observer) {
    return nullptr;
  }
  auto temp = gcnew MdfChannelObserver(observer.release());
  return temp;  
}

array<MdfChannelObserver^>^ MdfLibrary::CreateChannelObserverForChannelGroup(
    MdfDataGroup^ data_group, MdfChannelGroup^ channel_group) {
  if (data_group == nullptr || channel_group == nullptr ) {
    return nullptr;
  }
  if (data_group->group_ == nullptr || channel_group->group_ == nullptr) {
    return nullptr;
  }
  mdf::ChannelObserverList list;
  mdf::CreateChannelObserverForChannelGroup(*data_group->group_,
    *channel_group->group_, list);
  auto temp = gcnew array<MdfChannelObserver^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] =
      gcnew MdfChannelObserver(list[index].release());
  }
  return temp;  
}

String^ MdfLibrary::Utf8Conversion(const std::string& utf8_string) {
  array<byte>^ c_array = gcnew array<byte>(utf8_string.length());
  for (int i = 0; i < utf8_string.length(); i++) c_array[i] = utf8_string[i];

  System::Text::Encoding^ u8enc = System::Text::Encoding::UTF8;
  return u8enc->GetString(c_array);
}

void MdfLibrary::FireLogEvent(MdfLogSeverity severity, String^ function,
                              String^ message) {
  LogEvent(severity, function, message);
}

}
