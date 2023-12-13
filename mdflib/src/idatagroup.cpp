/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/idatagroup.h"

#include <algorithm>

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/isampleobserver.h"


namespace mdf {

void IDataGroup::AttachSampleObserver(ISampleObserver *observer) const {
  observer_list.emplace_back(observer);
}

void IDataGroup::DetachSampleObserver(const ISampleObserver *observer) const {
  if (observer_list.empty()) return;
  for (auto itr = observer_list.begin(); itr != observer_list.end();
       /* No ++itr here */) {
    if (*itr == observer) {
      itr = observer_list.erase(itr);
    } else {
      ++itr;
    }
  }
}

void IDataGroup::DetachAllSampleObservers() const { observer_list.clear(); }

void IDataGroup::NotifySampleObservers(
    size_t sample, uint64_t record_id,
    const std::vector<uint8_t> &record) const {
  for (auto *observer : observer_list) {
    if (observer != nullptr) {
      observer->OnSample(sample, record_id, record);
    }
  }
}

void IDataGroup::ResetSample() const {
  auto list = ChannelGroups();
  std::for_each(list.begin(),list.end(),
                        [](const auto *cg) { cg->ResetSample(); });
}

IMetaData *IDataGroup::CreateMetaData() { return nullptr; }

IMetaData *IDataGroup::MetaData() const { return nullptr; }

void IDataGroup::Description(const std::string &) {}

std::string IDataGroup::Description() const { return {}; }

void IDataGroup::RecordIdSize(uint8_t id_size) {}

uint8_t IDataGroup::RecordIdSize() const { return 0; }


bool IDataGroup::IsEmpty() const {
  // Check if any samples have been stored.
  const auto cg_list = ChannelGroups();
  return std::all_of(cg_list.cbegin(), cg_list.cend(), [] (const auto* group) {
    return group != nullptr &&  group->NofSamples() == 0;
  });
}

IChannelGroup *IDataGroup::CreateChannelGroup(const std::string_view &name) {
  auto cg_list = ChannelGroups();
  auto itr = std::find_if(cg_list.begin(), cg_list.end(),
                          [&] (const auto* group) {
    return group != nullptr && group->Name() == name;
  });
  if (itr != cg_list.end()) {
    return *itr;
  }
  auto* new_group = CreateChannelGroup();
  if (new_group != nullptr) {
    new_group->Name(name.data());
  }
  return new_group;
}

IChannelGroup *IDataGroup::GetChannelGroup(const std::string_view &name) const {
  auto cg_list = ChannelGroups();
  auto itr = std::find_if(cg_list.begin(), cg_list.end(),
      [&] (const auto* group) {
    return group != nullptr && group->Name() == name;
  });
  return itr != cg_list.end() ? *itr : nullptr;
}

IChannelGroup *IDataGroup::GetChannelGroup(uint64_t record_id) const {
  auto cg_list = ChannelGroups();
  auto itr = std::find_if(cg_list.begin(), cg_list.end(),
                [&] (const auto* group) {
                  return group != nullptr && group->RecordId() == record_id;
                });
  return itr != cg_list.end() ? *itr : nullptr;
}
}  // namespace mdf
