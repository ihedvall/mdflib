/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf4writer.h"

#include "mdf4file.h"
#include "platform.h"

namespace mdf::detail {

Mdf4Writer::~Mdf4Writer() { StopWorkThread(); }

IChannel* Mdf4Writer::CreateChannel(IChannelGroup* parent) {
  auto* cg4 = dynamic_cast<Cg4Block*>(parent);
  IChannel* cn = nullptr;
  if (cg4 != nullptr) {
    auto cn4 = std::make_unique<Cn4Block>();
    cn4->Init(*cg4);
    cg4->AddCn4(cn4);
    auto& cn_list = cg4->Cn4();
    cn = cn_list.empty() ? nullptr : cn_list.back().get();
  }
  return cn;
}

IChannelConversion* Mdf4Writer::CreateChannelConversion(IChannel* parent) {
  auto* cn4 = dynamic_cast<Cn4Block*>(parent);
  IChannelConversion* cc = nullptr;
  if (cn4 != nullptr) {
    auto cc4 = std::make_unique<Cc4Block>();
    cc4->Init(*cn4);
    cn4->AddCc4(cc4);
    cc = const_cast<Cc4Block*>(cn4->Cc());
  }
  return cc;
}

void Mdf4Writer::CreateMdfFile() {
  auto mdf4 = std::make_unique<Mdf4File>();
  mdf_file_ = std::move(mdf4);
}

void Mdf4Writer::SetLastPosition(std::FILE* file) {
  Platform::fseek64(file, 0, SEEK_END);

  auto* header = Header();
  if (header == nullptr) {
    return;
  }
  auto* last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(last_dg);
  if (dg4 == nullptr) {
    return;
  }

  if (dg4->Link(2) > 0) {
    return;
  }

  dg4->SetLastFilePosition(file);
  auto position = GetFilePosition(file);
  dg4->UpdateLink(file, 2, position);
  dg4->SetLastFilePosition(file);
}

}  // namespace mdf::detail