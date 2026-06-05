/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/iheader.h"
#include "mdf/imetadata.h"

namespace mdf {


void IHeader::MeasurementId(const std::string &uuid) {}

std::string IHeader::MeasurementId() const { return {}; }

void IHeader::RecorderId(const std::string &uuid) {}

std::string IHeader::RecorderId() const { return {}; }

void IHeader::RecorderIndex(int64_t index) {}

int64_t IHeader::RecorderIndex() const { return 0; }

void IHeader::StartAngle(double angle) {}

std::optional<double> IHeader::StartAngle() const { return {}; }

void IHeader::StartDistance(double distance) {}

std::optional<double> IHeader::StartDistance() const { return {}; }

IMetaData *IHeader::CreateMetaData() { return nullptr; }

IMetaData *IHeader::MetaData() const { return nullptr; }

IAttachment *IHeader::CreateAttachment() { return nullptr; }

std::vector<IAttachment *> IHeader::Attachments() const { return {}; }

IFileHistory *IHeader::CreateFileHistory() { return nullptr; }

std::vector<IFileHistory *> IHeader::FileHistories() const { return {}; }

IEvent *IHeader::CreateEvent() { return nullptr; }

std::vector<IEvent *> IHeader::Events() const { return {}; }

IChannelHierarchy *IHeader::CreateChannelHierarchy() { return nullptr; }

std::vector<IChannelHierarchy *> IHeader::ChannelHierarchies() const {
  return {};
}

void IHeader::SetHdComment(const HdComment &hd_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(hd_comment.ToXml());
  }
}

void IHeader::GetHdComment(HdComment &hd_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    hd_comment.FromXml(meta_data->XmlSnippet());
  }
}

void IHeader::CopyFrom(const IHeader &source) {
  if (!source.Author().empty()) {
    Author(source.Author());
  }
  if (!source.Department().empty()) {
    Department(source.Department());
  }
  if (!source.Project().empty()) {
    Project(source.Project());
  }
  if (!source.Subject().empty()) {
    Subject(source.Subject());
  }
  if (!source.Description().empty()) {
    Description(source.Description());
  }
  HdComment hd_comment;
  source.GetHdComment(hd_comment);
  SetHdComment(hd_comment);

  StartTime(source.StartTime());
  if (const auto start_angle = source.StartAngle();
      start_angle.has_value()) {
    StartAngle(start_angle.value());
  }
  if (const auto start_dist = source.StartDistance();
      start_dist.has_value()) {
    StartDistance(start_dist.value());
  }

}
}  // namespace mdf