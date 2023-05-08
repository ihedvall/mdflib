/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf3writer.h"

#include <chrono>
#include <cstdio>

#include "cc3block.h"
#include "cg3block.h"
#include "cn3block.h"
#include "dg3block.h"
#include "mdf/mdflogstream.h"
#include "mdf3file.h"
#include "platform.h"

using namespace std::chrono_literals;

namespace mdf::detail {

Mdf3Writer::~Mdf3Writer() { StopWorkThread(); }

IChannelConversion *Mdf3Writer::CreateChannelConversion(IChannel *parent) {
  return parent != nullptr ? parent->CreateChannelConversion() : nullptr;
}

void Mdf3Writer::CreateMdfFile() {
  auto mdf3 = std::make_unique<Mdf3File>();
  mdf_file_ = std::move(mdf3);
}

void Mdf3Writer::SetLastPosition(std::FILE *file) {
  Platform::fseek64(file, 0, SEEK_END);

  auto *header = Header();
  if (header == nullptr) {
    return;
  }
  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto *dg3 = dynamic_cast<Dg3Block *>(last_dg);
  if (dg3 == nullptr) {
    return;
  }

  if (dg3->Link(3) > 0) {
    return;
  }

  dg3->SetLastFilePosition(file);
  auto position = detail::GetFilePosition(file);
  dg3->UpdateLink(file, 3, position);
  dg3->SetLastFilePosition(file);
}

bool Mdf3Writer::PrepareForWriting() {
  auto *header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No header  found. Invalid use of the function.";
    return false;
  }

  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return true;
  }

  auto cg_list = last_dg->ChannelGroups();
  for (auto* group : cg_list) {
    if (group == nullptr) {
      continue;
    }
    auto* cg3 = dynamic_cast<Cg3Block*>(group);
    if (cg3 != nullptr) {
      cg3->PrepareForWriting();
    }
  }
  return true;
}


}  // namespace mdf::detail