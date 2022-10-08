/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf3writer.h"
#include "mdf/mdflogstream.h"
#include <chrono>
#include <cstdio>
#include <ranges>

#include "cc3block.h"
#include "cg3block.h"
#include "cn3block.h"
#include "dg3block.h"
#include "mdf3file.h"
#include "platform.h"

using namespace std::chrono_literals;

namespace mdf::detail {

Mdf3Writer::~Mdf3Writer() { StopWorkThread(); }

IChannel *Mdf3Writer::CreateChannel(IChannelGroup *parent) {
  auto *cg3 = dynamic_cast<detail::Cg3Block *>(parent);
  if (cg3 != nullptr) {
    auto cn3 = std::make_unique<detail::Cn3Block>();
    cn3->Init(*cg3);
    cg3->AddCn3(cn3);
  }
  return cg3 != nullptr ? cg3->Cn3().back().get() : nullptr;
}

IChannelConversion *Mdf3Writer::CreateChannelConversion(IChannel *parent) {
  auto *cn3 = dynamic_cast<detail::Cn3Block *>(parent);
  if (cn3 != nullptr) {
    auto cc3 = std::make_unique<detail::Cc3Block>();
    cc3->Init(*cn3);
    cn3->AddCc3(cc3);
  }
  return cn3 != nullptr ? cn3->Cc3() : nullptr;
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

} // namespace mdf::detail