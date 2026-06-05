/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/isourceinformation.h"

namespace mdf {

void ISourceInformation::SetSiComment(const SiComment &si_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(si_comment.ToXml());
  }
}

void ISourceInformation::GetSiComment(SiComment &si_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    si_comment.FromXml(meta_data->XmlSnippet());
  }
}

void ISourceInformation::CopyFrom(const ISourceInformation& source) {
  Type(source.Type());
  Bus(source.Bus());
  Flags(source.Flags());
  Name(source.Name());
  Path(source.Path());
  SiComment si_comment;
  source.GetSiComment(si_comment);
  SetSiComment(si_comment);
}

}  // namespace mdf