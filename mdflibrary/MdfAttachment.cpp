/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <msclr/marshal_cppstd.h>

#include "MdfAttachment.h"

using namespace msclr::interop;
using namespace System;

namespace MdfLibrary {

MdfAttachment::MdfAttachment(mdf::IAttachment* attachment) {
  attachment_ = attachment;
}

Int64 MdfAttachment::Index::get() {
  return attachment_ != nullptr ? attachment_->Index() : 0;
}

unsigned short MdfAttachment::CreatorIndex::get() {
  return attachment_ != nullptr ? attachment_->CreatorIndex() : 0;
}

void MdfAttachment::CreatorIndex::set(unsigned short index) {
  if (attachment_ != nullptr) {
    attachment_->CreatorIndex(index);
  }
}

bool MdfAttachment::Embedded::get() {
  return attachment_ != nullptr ? attachment_->IsEmbedded() : false;  
}

void MdfAttachment::Embedded::set(bool embedded) {
  if (attachment_ != nullptr) {
    attachment_->IsEmbedded(embedded);
  }  
}

bool MdfAttachment::Compressed::get() {
   return attachment_ != nullptr ? attachment_->IsCompressed() : false;   
}

void MdfAttachment::Compressed::set(bool compressed) {
  if (attachment_ != nullptr) {
    attachment_->IsCompressed(compressed);
  }  
}

String^ MdfAttachment::Md5::get() {
  if (attachment_ == nullptr) {
    return gcnew String("");
  }
  
  const std::string temp = attachment_->Md5().has_value() ?
    attachment_->Md5().value() : std::string();
  return gcnew String(temp.c_str());    
}

String^ MdfAttachment::Filename::get() {
  const std::string temp = attachment_ != nullptr ?
  attachment_->FileName() : std::string();
  return gcnew String(temp.c_str()); 
}

void MdfAttachment::Filename::set(String^ filename) {
  const auto temp = String::IsNullOrEmpty(filename) ?
    std::string() : marshal_as<std::string>(filename);
  if (attachment_ != nullptr) {
    attachment_->FileName(temp);
  }
}

String^ MdfAttachment::FileType::get() {
  const std::string temp = attachment_ != nullptr ?
  attachment_->FileType() : std::string();
  return gcnew String(temp.c_str());   
}

void MdfAttachment::FileType::set(String^ type) {
  const auto temp = String::IsNullOrEmpty(type) ?
    std::string() : marshal_as<std::string>(type);
  if (attachment_ != nullptr) {
    attachment_->FileName(temp);
  }
}

}




