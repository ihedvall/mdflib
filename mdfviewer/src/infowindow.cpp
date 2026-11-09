/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "infowindow.h"

#include <filesystem>

#include <wx/msgdlg.h>
#include <wx/splitter.h>

#include "mdf/mdflogstream.h"
#include "util/timestamp.h"

#include "displaywindow.h"
#include "mdfdocument.h"
#include "mdf4file.h"
#include "mdf3file.h"
#include "configwindow.h"

using namespace mdf::detail;
using namespace std::filesystem;
namespace {
const BlockProperty* FindProperty(const BlockPropertyList& property_list,
                        const std::string& label) {
  for (const BlockProperty& property : property_list) {
    if (property.Label() == label) {
      return &property;
    }
  }
  return nullptr;
}

}

namespace mdf::viewer {

InfoWindow::InfoWindow(wxWindow* parent, wxWindowID id)
: wxListView(parent, id,wxDefaultPosition, wxDefaultSize,
             wxLC_REPORT | wxLC_SINGLE_SEL) {
  Bind(wxEVT_LIST_ITEM_ACTIVATED, &InfoWindow::OnListItemActivated,
    this);
}

MdfDocument* InfoWindow::GetDoc() const {
  DisplayWindow* parent = dynamic_cast<DisplayWindow*>(GetParent());
  if (parent == nullptr) {
    return nullptr;
  }
  return parent->GetDoc();
}

const IHeader* InfoWindow::GetHeader() const {
  const MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return nullptr;
  }
  const ConfigData& data = doc->GetLastConfigData();
  const MdfReader* reader = data.Reader();
  if (reader == nullptr) {
    return nullptr;
  }
  const MdfFile* file = reader->GetFile();
  if (file == nullptr) {
    return nullptr;
  }
  return file->Header();
}

const MdfBlock* InfoWindow::GetCurrentBlock() const {
  const MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return nullptr;
  }
  const ConfigData& data = doc->GetLastConfigData();
  const int64_t block_index = data.BlockIndex();
  return GetBlock(block_index);
}

const MdfBlock* InfoWindow::GetBlock(int64_t block_index) const {
  const MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return nullptr;
  }
  const ConfigData& data = doc->GetLastConfigData();
  const MdfReader* reader = data.Reader();
  if (reader == nullptr) {
    return nullptr;
  }
  return doc->GetBlock(reader, block_index);

}

ConfigWindow* InfoWindow::GetConfigWindow() const {
  wxWindow* notebook = GetParent();
  if (notebook == nullptr) {
    return nullptr;
  }

  auto* splitter = dynamic_cast<wxSplitterWindow*>(notebook->GetParent());
  if (splitter == nullptr) {
    return nullptr;
  }
  return dynamic_cast<ConfigWindow*>(splitter->GetWindow1());
}

void InfoWindow::Redraw() {
  ClearAll(); // Remove all items and headers
  MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const ConfigData& data = doc->GetLastConfigData();
  switch (data.Position()) {
    case ConfigPosition::kConfig:
      RedrawConfig();
      break;

    case ConfigPosition::kHistories:
      RedrawHistory();
      break;

    case ConfigPosition::kMeasurements:
      RedrawMeasurement();
      break;

    case ConfigPosition::kEvents:
      RedrawEvent();
      break;

    case ConfigPosition::kAttachments:
      RedrawAttachment();
      break;

    case ConfigPosition::kHierarchies:
      RedrawHierarchy();
      break;

    case ConfigPosition::kMdfBlock:
      RedrawBlock();
      break;

    case ConfigPosition::kSignal:
      RedrawBlock();
      break;

    default:
      break;
  }
}

void InfoWindow::RedrawConfig() {
  MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const plot::Configuration& config = doc->GetConfiguration();
  BlockPropertyList list;
  list.emplace_back("General", "", "", BlockItemType::HeaderItem);
  list.emplace_back("Display Name", config.GetName());
  list.emplace_back("Description", config.GetDescription());
  try {
    const path fullname(config.GetFilename());
    const path filename = fullname.filename();
    const path parent_path = fullname.parent_path();
    list.emplace_back("File Name", filename.string());
    list.emplace_back("File Path", parent_path.string());
  } catch (const std::exception& error) {
    MDF_ERROR() << "File path error. Error: " << error.what();
  }
  DrawPropertyList(list);
}

void InfoWindow::RedrawHistory() {
  AppendColumn("Time", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("User", wxLIST_FORMAT_LEFT, 100);
  AppendColumn("Description", wxLIST_FORMAT_LEFT, 200);
  AppendColumn("Tool ID", wxLIST_FORMAT_LEFT, 100);
  AppendColumn("Tool Vendor", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("Tool Version", wxLIST_FORMAT_LEFT, 100);

  const auto* header = GetHeader();
  if (header == nullptr) {
    return;
  }
  const auto history_list = header->FileHistories();
  long line = 0;
  for (const auto* history : history_list) {
    if (history == nullptr) {
      continue;
    }
    std::ostringstream date_time;
    date_time << util::time::NsToLocalDate(history->Time()) << " "
              << util::time::NsToLocalTime(history->Time(), 0);
    const auto index = InsertItem(line, wxString::FromUTF8(date_time.str()));
    SetItem(index, 1, wxString::FromUTF8(history->UserName()));
    SetItem(index, 2, wxString::FromUTF8(history->Description()));
    SetItem(index, 3, wxString::FromUTF8(history->ToolName()));
    SetItem(index, 4, wxString::FromUTF8(history->ToolVendor()));
    SetItem(index, 5, wxString::FromUTF8(history->ToolVersion()));
    ++line;
  }
}

void InfoWindow::RedrawMeasurement() {
  AppendColumn("Measurement", wxLIST_FORMAT_LEFT, 100);
  AppendColumn("Samples", wxLIST_FORMAT_LEFT, 100);
  AppendColumn("Description", wxLIST_FORMAT_LEFT, 400);
  AppendColumn("Channel Groups", wxLIST_FORMAT_LEFT, 400);

  const auto* header = GetHeader();
  if (header == nullptr) {
    return;
  }

  const auto measurement_list = header->DataGroups();
  long line = 0;
  for (const auto* meas : measurement_list) {
    if (meas == nullptr) {
      continue;
    }
    const auto group_list = meas->ChannelGroups();
    const auto* group = group_list.size() == 1 ? group_list.front() : nullptr;
    // Insert fictive measurement number
    const auto index = InsertItem(line, std::to_string(line + 1));
    uint64_t samples = 0;
    std::ostringstream groups;
    for ( const auto* group1 : group_list) {
      if (!groups.str().empty()) {
        groups <<",";
      }
      groups << group1->Name();
      samples += group1->NofSamples();
    }
    // Measurement description
    std::ostringstream desc;
    if (!meas->Description().empty()) {
      desc << meas->Description();
      if (group != nullptr) {
        desc << ", Samples: " << group->NofSamples();
      }
    } else if (group != nullptr) {
      desc << group->Name();
    } else {
      desc << "Measurement " << line + 1;
    }
    SetItem(index, 1, std::to_string(samples));
    SetItem(index, 2, wxString::FromUTF8(desc.str()));
    SetItem(index, 3, wxString::FromUTF8(groups.str()));
    ++line;
  }
}

void InfoWindow::RedrawEvent() {
  AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("Type", wxLIST_FORMAT_LEFT, 75);
  AppendColumn("Value", wxLIST_FORMAT_LEFT, 75);
  AppendColumn("Description", wxLIST_FORMAT_LEFT, 200);
  AppendColumn("Range", wxLIST_FORMAT_LEFT, 75);
  AppendColumn("Cause", wxLIST_FORMAT_LEFT, 75);
  const auto* header = GetHeader();
  if (header == nullptr) {
    return;
  }

  const auto event_list = header->Events();
  long line = 0;
  for (const auto* event : event_list) {
    if (event == nullptr) {
      continue;
    }
    const auto index = InsertItem(line, wxString::FromUTF8(event->Name()));
    SetItem(index, 1, event->TypeToString());
    SetItem(index, 2, event->ValueToString());
    SetItem(index, 3, wxString::FromUTF8(event->Description()));
    SetItem(index, 4, event->RangeToString());
    SetItem(index, 5, event->CauseToString());
    ++line;
  }
}

void InfoWindow::RedrawAttachment() {
  AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("Embedded", wxLIST_FORMAT_LEFT, 100);
  AppendColumn("Type", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("Path", wxLIST_FORMAT_LEFT, 300);
  const auto* header = GetHeader();
  if (header == nullptr) {
    return;
  }

  const auto attachment_list = header->Attachments();
  long line = 0;
  for (const auto* attachment : attachment_list) {
    if (attachment == nullptr) {
      continue;
    }
    wxString name;
    wxString path;
    try {
      wxString filename = wxString::FromUTF8(attachment->FileName());
      std::filesystem::path fullname(filename.ToStdWstring());
      name = fullname.filename().wstring();
      path = fullname.parent_path().wstring();
    } catch (const std::exception& error) {
      MDF_ERROR() << "Invalid path error. Error: " << error.what();
    }
    const auto index = InsertItem(line, name);
    SetItem(index, 1, attachment->IsEmbedded() ? "Yes" : "");
    SetItem(index, 2, wxString::FromUTF8(attachment->FileType()));
    SetItem(index, 3, path);
    ++line;
  }

}

void InfoWindow::RedrawHierarchy() {
  AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("Type", wxLIST_FORMAT_LEFT, 200);
  const MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const ConfigData& config = doc->GetLastConfigData();
  const MdfReader* reader = config.Reader();
  if (reader == nullptr) {
    return;
  }
  const MdfFile* file = reader->GetFile();
  if (file == nullptr) {
    return;
  }
  const auto* header = file->Header();
  if (header == nullptr) {
    return;
  }
  const auto hierarchy_list = header->ChannelHierarchies();
  long line = 0;
  for (const auto* hierarchy : hierarchy_list) {
    if (hierarchy == nullptr) {
      continue;
    }
    const auto index = InsertItem(line, wxString::FromUTF8(hierarchy->Name()));
    SetItem(index, 1, hierarchy->TypeToString());
    ++line;
  }

}

void InfoWindow::RedrawBlock() {
  MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const ConfigData& config = doc->GetLastConfigData();
  MdfReader* reader = config.Reader();
  if (reader == nullptr) {
    return;
  }
  const MdfFile* file = reader->GetFile();
  if (file == nullptr) {
    return;
  }
  const MdfBlock* block = nullptr;
  const int64_t block_index = config.BlockIndex();

  if (const auto* mdf4_file = dynamic_cast<const Mdf4File*>(file);
      mdf4_file != nullptr ) {
    block = mdf4_file->Find(block_index);
  } else if (const auto* mdf3_file = dynamic_cast<const Mdf3File*>(file);
      mdf3_file != nullptr ) {
    block = mdf3_file->Find(block_index);
  }

  if (block == nullptr) {
    return;
  }

  BlockPropertyList property_list;
  block->GetBlockProperty(property_list);
  DrawPropertyList(property_list);
}

void InfoWindow::DrawPropertyList(const BlockPropertyList& property_list) {
  AppendColumn("Property", wxLIST_FORMAT_LEFT, 150);
  AppendColumn("Value", wxLIST_FORMAT_LEFT, 200);
  AppendColumn("Description", wxLIST_FORMAT_LEFT, 200);

  long line = 0;
  for (const auto& prop : property_list) {
    switch (prop.Type()) {
      case BlockItemType::HeaderItem:
      {
        auto index = InsertItem(line, wxString::FromUTF8(prop.Label()));
        auto font = GetItemFont(index);
        font.MakeItalic();
        SetItemFont(index, font);
      }
      break;

      case BlockItemType::LinkItem:
      {
        auto index = InsertItem(line, wxString::FromUTF8(prop.Label()));
        if (prop.Link() > 0) {
          SetItem(index, 1, wxString::FromUTF8(prop.Value()));
          SetItem(index, 2, wxString::FromUTF8(prop.Description()));
        } else if (!prop.Description().empty()) {
          SetItem(index, 2, wxString::FromUTF8(prop.Description()));
        }
      }
      break;

      case BlockItemType::BlankItem:
      {
        auto index = InsertItem(line, "");
        SetItem(index, 1, "");
        SetItem(index, 2, "");
      }
      break;

      default:
      {
        auto index = InsertItem(line, wxString::FromUTF8(prop.Label()));
        SetItem(index, 1, wxString::FromUTF8(prop.Value()));
        SetItem(index, 2, wxString::FromUTF8(prop.Description()));
      }
      break;

    }
    ++line;
  }

}

void InfoWindow::OnListItemActivated(wxListEvent& event) {
  const long index = event.GetIndex();
  const MdfBlock* current = GetCurrentBlock();
  const MdfDocument* doc = GetDoc();
  if (index < 0 || current == nullptr || doc == nullptr) {
    return;
  }
  const ConfigData& data = doc->GetLastConfigData();
  const MdfReader* reader = data.Reader();
  if (reader == nullptr) {
    return;
  }
  BlockPropertyList property_list;
  current->GetBlockProperty(property_list);
  const std::string label = event.GetText().utf8_string();
  const BlockProperty* property = FindProperty(property_list, label);
  if (property == nullptr) {
    return;
  }
  const int64_t link_index = property->Link();
  if (link_index <= 0) {
    return;
  }
  const MdfBlock* link = GetBlock(link_index);
  if (link == nullptr) {
    return;
  }
  if (link->BlockType() == "TX") {
    if (link->IsMdf4()) {
      if (const auto* tx = dynamic_cast<const Tx4Block*>(link);
          tx != nullptr) {
        wxMessageBox(wxString::FromUTF8(tx->Text()),
          "TX Block (Text)",
          wxICON_INFORMATION);
      }
    } else {
      if (const auto* tx = dynamic_cast<const Tx3Block*>(link);
          tx != nullptr) {
        wxMessageBox(wxString::FromUTF8(tx->Text()), "TX Block (Text)",
          wxICON_INFORMATION);
      }
    }
    return;
  }

  if (link->BlockType() == "MD") {
    // Show the XML in a dialog window
    if (const auto* md = dynamic_cast<const Md4Block*>(link);
        md != nullptr) {
      wxMessageBox(wxString::FromUTF8(md->Text()), "MD block (XML)",
                    wxICON_INFORMATION);
    }
    return;
  }

  if (link->BlockType() == "PR") {
    // Show the XML in a dialog window
    const auto* pr3 = dynamic_cast<const Pr3Block*>(link);
    if (pr3 != nullptr) {
      wxMessageBox(wxString::FromUTF8(pr3->Text()), "PR block",
        wxICON_INFORMATION);
    }
    return;
  }

  ConfigWindow* config_window = GetConfigWindow();
  if (config_window == nullptr) {
    return;
  }
  config_window->SelectBlock(reader,  link_index);
}

} // mdf::viewer