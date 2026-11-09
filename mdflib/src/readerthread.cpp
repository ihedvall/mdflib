/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/readerthread.h"

#include <filesystem>
#include <memory>

#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdflogstream.h"

#include "mdfsignal.h"

using namespace std::filesystem;

namespace mdf {

bool ReaderThread::AddMdfFile(const std::wstring& filename) {
  std::string fname;
  bool add = false;
  try {
    path fullname(filename);
    fname = fullname.string();
    if (filename.empty()) {
      throw std::invalid_argument("The filename cannot be empty");
    }
    if (!exists(fullname)) {
      throw std::runtime_error("The file does not exist");
    }
    auto reader = std::make_unique<MdfReader>(fullname);
    if ( !reader->IsOk()) {
      throw std::runtime_error("The file is not a valid MDF file");
    }
    const bool info = reader->ReadEverythingButData();
    if (!info) {
      throw std::runtime_error("Failed to read the configuration");
    }
    reader->Close();
    CreateUniqueName(*reader);
    const bool signals = CreateSignals(*reader);
    if (!signals) {
      throw std::runtime_error("Failed to create signals");
    }
    reader_list_.emplace_back(std::move(reader));
    add = true;
  } catch (const std::exception& error) {
    MDF_ERROR() << "Failed to add MDF file. Error: " << error.what()
      << ", File: " << fname;
  }
  return add;
}

void ReaderThread::CreateUniqueName(MdfReader& reader) const {
  try {
    const path fullname(reader.WFilename());
    const std::wstring base_name = fullname.stem().wstring();
    std::wstring name = base_name;
    bool unique = false;
    for (size_t index = 1; index < 100; ++index) {
      const auto itr = std::find_if(
        reader_list_.cbegin(), reader_list_.cend(),
        [name] (const auto& reader1) -> bool {
          const MdfReader* reader2 = reader1.get();
          if (reader2 == nullptr) {
            return false;
          }
          return reader2->GetName() == name;
      });

      if (itr == reader_list_.cend()) {
        unique = true;
        break;
      }
      name = base_name + L"_" + std::to_wstring(index);
    }
    if (!unique) {
      throw std::runtime_error("Failed to create an unique name for the MDF file.");
    }
    reader.SetName(name);
  } catch (const std::exception& error) {
    const std::string fname = reader.Filename();
    MDF_ERROR() << "Failed to create an unique name for the MDF file. Error:  "
      << error.what() << ", File: " << fname;
  }
}

bool ReaderThread::CreateSignals(MdfReader& reader) {
  bool create = false;
  try {
    const MdfFile* file = reader.GetFile();
    if (file == nullptr) {
      throw std::runtime_error("Missing file");
    }
    const IHeader* header = file->Header();
    if (header == nullptr) {
      throw std::runtime_error("Missing header block (HD)");
    }
    const auto dg_list = header->DataGroups();
    for (IDataGroup* data_group : dg_list) {
      if (data_group == nullptr) {
        continue;
      }
      const auto cg_list = data_group->ChannelGroups();
      for (const IChannelGroup* channel_group : cg_list) {
        if (channel_group == nullptr) {
          continue;
        }
        const auto cn_list = channel_group->Channels();
        for (const IChannel* channel : cn_list) {
          if (channel == nullptr) {
            continue;
          }
          auto signal = std::make_unique<MdfSignal>(&reader, data_group,
            channel_group, channel);
          signal_list_.emplace_back(std::move(signal));
        }
      }
    }
    create = true;
  } catch (const std::exception& error) {
    MDF_ERROR() << "Failed to create signals. Error: " << error.what();
  }
  return create;
}

} // mdf