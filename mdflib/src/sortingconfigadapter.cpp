/*
* Copyright 2026 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "sortingconfigadapter.h"

#include "mdf/ifilehistory.h"

namespace mdf {
SortingConfigAdapter::SortingConfigAdapter(const MdfWriter& writer,
                                           MdfReader& reader) :
  IConfigAdapter(writer),
  reader_(reader) {

}

void SortingConfigAdapter::CreateConfig(IHeader& header) {
  const IHeader* source_header = reader_.GetHeader();
  if (source_header == nullptr) {
    return;
  }
  header.CopyFrom(*source_header);
  for (const IAttachment* source_attachment : source_header->Attachments()) {
    if (source_attachment == nullptr) {
      continue;
    }
    if (IAttachment* attachment = header.CreateAttachment();
        attachment != nullptr ) {
      attachment->CopyFrom(*source_attachment, reader_);
    }
  }

  for (const IFileHistory* source_history : source_header->FileHistories()) {
    if (source_history == nullptr) {
      continue;
    }
    if (IFileHistory* history = header.CreateFileHistory();
        history != nullptr ) {
      history->CopyFrom(*source_history);
    }
  }

  // Events are tricky to import as they reference MDF objects that is moved
  // Channel hierarchy are tricky to import as the hierachy is modified.
}

} // mdf
