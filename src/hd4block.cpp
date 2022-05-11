/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "hd4block.h"
#include "util/ixmlfile.h"

using namespace util::xml;

namespace {
  // LINK index
constexpr size_t kIndexDg = 0;
constexpr size_t kIndexFh = 1;
constexpr size_t kIndexCh = 2;
constexpr size_t kIndexAt = 3;
constexpr size_t kIndexEv = 4;
constexpr size_t kIndexMd = 5;
constexpr size_t kIndexNext = 0;

template <typename T>
T GetCommonProperty(const mdf::detail::Hd4Block& block, const std::string &key) {
  const auto* md4 = block.Md4();
  if (md4 == nullptr || md4->IsTxtBlock()) {
    return {};
  }
  auto xml_file = util::xml::CreateXmlFile("Expat");
  xml_file->ParseString(md4->Text());
  const auto* common = xml_file->GetNode("common_properties");
  if (common == nullptr) {
    return {};
  }
  const auto* key_node = common->GetNode("e", "name", key);
  return key_node == nullptr ? T {} : key_node->Value<T>();
}

template <typename T>
void SetCommonProperty(mdf::detail::Hd4Block& block, const std::string &key, const T &value) {
  auto xml_file = util::xml::CreateXmlFile("Expat");
  const auto* md4 = block.Md4();
  if (md4 != nullptr) {
    xml_file->ParseString(md4->Text());
  }
  auto& root = xml_file->RootName("HDcomment");
  auto& common = root.AddUniqueNode("common_properties");
  auto& key_node = root.AddUniqueNode("e", "name", key);
  if (typeid(T) == typeid(int64_t)) {
    key_node.SetAttribute<std::string>("type", "integer");
  } else if (typeid(T) == typeid(std::string)) {
    key_node.SetAttribute<std::string>("type", "string");
  }
  key_node.Value(value);
  block.Md4(xml_file->WriteString(true));
}

}

namespace mdf::detail {

const IBlock *Hd4Block::Find(fpos_t index) const {
  for (const auto& dg : dg_list_) {
    if (!dg) {
      continue;
    }
    const auto* p = dg->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (const auto& fh : fh_list_) {
    if (!fh) {
      continue;
    }
    const auto* p = fh->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (const auto& ch : ch_list_) {
    if (!ch) {
      continue;
    }
    const auto* p = ch->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (const auto& at : at_list_) {
    if (!at) {
      continue;
    }
    const auto* p = at->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (const auto& ev : ev_list_) {
    if (!ev) {
      continue;
    }
    const auto* p = ev->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  return IBlock::Find(index);
}

void Hd4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("First DG", ToHexString(Link(kIndexDg)), "Link to first data group", BlockItemType::LinkItem );
  dest.emplace_back("First FH", ToHexString(Link(kIndexFh)), "Link to first file history",BlockItemType::LinkItem );
  dest.emplace_back("First CH", ToHexString(Link(kIndexCh)), "Link to first channel hierarchy", BlockItemType::LinkItem );
  dest.emplace_back("First AT", ToHexString(Link(kIndexAt)), "Link to first attachment",  BlockItemType::LinkItem );
  dest.emplace_back("First EV", ToHexString(Link(kIndexEv)), "Link to first event",BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  timestamp_.GetBlockProperty(dest);
  switch (static_cast<Hd4TimeClass>(time_class_)) {
    case Hd4TimeClass::LocalPcTime:
      dest.emplace_back("Time Source", "PC Time");
      break;

    case Hd4TimeClass::ExternalTime:
      dest.emplace_back("Time Source", "External");
      break;

    case Hd4TimeClass::ExternalNTP:
      dest.emplace_back("Time Source", "NTP/PTP");
      break;

  }
  dest.emplace_back("Flags", ToHexString(flags_));
  dest.emplace_back("Start Angle [rad]", ToString(start_angle_));
  dest.emplace_back("Start Distance [m]", ToString(start_distance_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Hd4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);

  timestamp_.Init(*this);
  bytes += timestamp_.Read(file);

  bytes += ReadNumber(file, time_class_);
  bytes += ReadNumber(file, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 1);
  bytes += ReadNumber(file, start_angle_);
  bytes += ReadNumber(file, start_distance_);

  ReadMdComment(file, kIndexMd);

  return bytes;
}

void Hd4Block::ReadMeasurementInfo(std::FILE *file) {
    // We assume that the ID and HD block have been read (see ReadHeader)
  if (dg_list_.empty() && Link(kIndexDg) > 0) {
    for (auto link = Link(kIndexDg); link > 0; /* No ++ here*/) {
      auto dg = std::make_unique<Dg4Block>();
      dg->Init(*this);
      SetFilePosition(file, link);
      dg->Read(file);
      dg->ReadCgList(file);
      link = dg->Link(kIndexNext);
      dg_list_.emplace_back(std::move(dg));
    }
  }

  if (fh_list_.empty() && Link(kIndexFh) > 0) {
    for (auto link = Link(kIndexFh); link > 0; /* No ++ here*/) {
      auto fh = std::make_unique<Fh4Block>();
      fh->Init(*this);
      SetFilePosition(file, link);
      fh->Read(file);
      link = fh->Link(kIndexNext);
      fh_list_.emplace_back(std::move(fh));
    }
  }

  if (ch_list_.empty() && Link(kIndexCh) > 0) {
    for (auto link = Link(kIndexCh); link > 0; /* No ++ here*/) {
      auto ch = std::make_unique<Ch4Block>();
      ch->Init(*this);
      SetFilePosition(file, link);
      ch->Read(file);
      link = ch->Link(kIndexNext);
      ch_list_.emplace_back(std::move(ch));
    }
  }

  if (at_list_.empty() && Link(kIndexAt) > 0) {
    for (auto link = Link(kIndexAt); link > 0; /* No ++ here*/) {
      auto at = std::make_unique<At4Block>();
      at->Init(*this);
      SetFilePosition(file, link);
      at->Read(file);
      link = at->Link(kIndexNext);
      at_list_.emplace_back(std::move(at));
    }
  }

  if (ev_list_.empty() && Link(kIndexEv) > 0) {
    for (auto link = Link(kIndexEv); link > 0; /* No ++ here*/) {
      auto ev = std::make_unique<Ev4Block>();
      ev->Init(*this);
      SetFilePosition(file, link);
      ev->Read(file);
      link = ev->Link(kIndexNext);
      ev_list_.emplace_back(std::move(ev));
    }
  }

}

void Hd4Block::ReadEverythingButData(std::FILE *file) {
  // We assume that ReadMeasurementInfo have been called earlier
  for ( auto& dg : dg_list_) {
    if (!dg) {
      continue;
    }
    for (auto& cg : dg->Cg4()) {
      cg->ReadCnList(file);
      cg->ReadSrList(file);
    }
  }
}

int64_t Hd4Block::Index() const {
  return FilePosition();
}

void Hd4Block::Author(const std::string &author) {
  SetCommonProperty(*this, "author", author);
}

std::string Hd4Block::Author() const {
  return GetCommonProperty<std::string>(*this, "author");
}

void Hd4Block::Department(const std::string &department) {
  SetCommonProperty(*this, "department", department);
}

std::string Hd4Block::Department() const {
  return GetCommonProperty<std::string>(*this, "department");
}

void Hd4Block::Project(const std::string &name) {
  SetCommonProperty(*this, "project", name);
}

std::string Hd4Block::Project() const {
  return GetCommonProperty<std::string>(*this, "project");
}

void Hd4Block::Subject(const std::string &subject) {
  SetCommonProperty(*this, "subject", subject);
}

std::string Hd4Block::Subject() const {
  return GetCommonProperty<std::string>(*this, "subject");
}

void Hd4Block::Description(const std::string &description) {
  auto xml_file = util::xml::CreateXmlFile("Expat");
  const auto* md4 = Md4();
  if (md4 != nullptr) {
    xml_file->ParseString(md4->Text());
  }
  auto& root = xml_file->RootName("HDcomment");
  auto& tx_node = root.AddUniqueNode("TX");
  tx_node.Value(description);
  Md4(xml_file->WriteString(true));
}

std::string Hd4Block::Description() const {
  return Comment();
}

void Hd4Block::MeasurementId(const std::string &uuid) {
  SetCommonProperty(*this, "Measurement.UUID", uuid);
}

std::string Hd4Block::MeasurementId() const {
  return GetCommonProperty<std::string>(*this, "Measurement.UUID");
}

void Hd4Block::RecorderId(const std::string &uuid) {
  SetCommonProperty(*this,"Recorder.UUID", uuid);
}

std::string Hd4Block::RecorderId() const {
  return GetCommonProperty<std::string>(*this,"Recorder.UUID");
}

void Hd4Block::RecorderIndex(int64_t index) {
  SetCommonProperty(*this,"Recorder.FileIndex", index);
}

int64_t Hd4Block::RecorderIndex() const {
  return GetCommonProperty<int64_t>(*this,"Recorder.FileIndex");
}

void Hd4Block::StartTime(uint64_t ns_since_1970) {
  timestamp_.NsSince1970(ns_since_1970);
}

uint64_t Hd4Block::StartTime() const {
  return timestamp_.NsSince1970();
}

void Hd4Block::MetaData(const std::string &meta_data) {
  Md4(meta_data);
}

std::string Hd4Block::MetaData() const {
  const auto* md4 = Md4();
  return md4 == nullptr ?  std::string() : md4->Text();
}

IDataGroup *Hd4Block::LastDataGroup() const {
  return dg_list_.empty() ? nullptr : dg_list_.back().get();
}

std::vector<IDataGroup *> Hd4Block::DataGroups() const {
  std::vector<IDataGroup *> list;
  std::ranges::for_each(dg_list_, [&] (const auto& dg) { list.push_back(dg.get());  });
  return list;
}





}