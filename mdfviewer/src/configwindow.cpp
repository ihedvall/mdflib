/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

// ReSharper disable CppUnusedIncludeDirective
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "configwindow.h"

#include <filesystem>
#include <sstream>

#include <wx/sizer.h>

#include "mdf/mdflogstream.h"

#include "configdata.h"
#include "childframe.h"

#include "dz4block.h"
#include "dl4block.h"
#include "ca4block.h"
#include "windowid.h"

using namespace plot;
using namespace std::filesystem;
using namespace mdf::detail;

namespace {

#include "img/at_block.cpp"
#include "img/at_root.cpp"
#include "img/ca_block.cpp"
#include "img/cc_block.cpp"
#include "img/cg_block.cpp"
#include "img/ch_block.cpp"
#include "img/ch_root.cpp"
#include "img/cn_block.cpp"
#include "img/config.cpp"
#include "img/database.cpp"
#include "img/dg_block.cpp"
#include "img/dg_root.cpp"
#include "img/dl_block.cpp"
#include "img/document_text.cpp"
#include "img/dt_block.cpp"
#include "img/dz_block.cpp"
#include "img/ev_block.cpp"
#include "img/ev_root.cpp"
#include "img/fh_block.cpp"
#include "img/fh_root.cpp"
#include "img/folder_close.cpp"
#include "img/folder_open.cpp"
#include "img/hd_block.cpp"
#include "img/hl_block.cpp"
#include "img/id_block.cpp"
#include "img/layers.cpp"
#include "img/lineplot.cpp"
#include "img/memory.cpp"
#include "img/mf4.cpp"
#include "img/rd_block.cpp"
#include "img/sd_block.cpp"
#include "img/settings.cpp"
#include "img/si_block.cpp"
#include "img/sr_block.cpp"

#include "img/resources32.cpp"
#include "img/resources24.cpp"
#include "img/resources16.cpp"

#include "img/variables32.cpp"
#include "img/variables24.cpp"
#include "img/variables16.cpp"

constexpr int kBmpConfig = 0;
constexpr int kBmpLayers = 1;
constexpr int kBmpLinePlot = 2;
constexpr int kBmpDatabase = 3;
constexpr int kBmpDocText = 4;
// constexpr int kBmpFolderClose = 5;
constexpr int kBmpFolderOpen = 6;
constexpr int kBmpMemory = 7;
constexpr int kBmpMf4 = 8;
constexpr int kBmpSettings = 9;

constexpr int kBmpAtBlock = 10;
constexpr int kBmpAtRoot = 11;
constexpr int kBmpCaBlock = 12;
constexpr int kBmpCcBlock = 13;
constexpr int kBmpCgBlock = 14;
constexpr int kBmpChBlock = 15;
constexpr int kBmpChRoot = 16;
constexpr int kBmpCnBlock = 17;
constexpr int kBmpDgBlock = 18;
constexpr int kBmpDgRoot = 19;
constexpr int kBmpDlBlock = 20;
constexpr int kBmpDtBlock = 21;
constexpr int kBmpDzBlock = 22;
constexpr int kBmpEvBlock = 23;
constexpr int kBmpEvRoot = 24;
constexpr int kBmpFhBlock = 25;
constexpr int kBmpFhRoot = 26;
constexpr int kBmpHdBlock = 27;
constexpr int kBmpHlBlock = 28;
constexpr int kBmpIdBlock = 29;
constexpr int kBmpRdBlock = 30;
constexpr int kBmpSdBlock = 31;
constexpr int kBmpSiBlock = 32;
constexpr int kBmpSrBlock = 33;

constexpr int kBmpResources = 0;
constexpr int kBmpVariables = 1;

constexpr size_t kConfigurationPage = 0;
constexpr size_t kSignalPage = 1;

wxString CreateBlockText(const MdfBlock& block) {
  std::ostringstream block_string;
  block_string << block.BlockType();
  std::string comment = block.Comment();

  if (comment.size() > 20) {
    // Truncate the comment
    comment = comment.substr(0, 20) + "...";
  }
  if (!comment.empty()) {
    block_string << " (" << comment << ")";
  }
  return wxString::FromUTF8(block_string.str());
}
}

namespace mdf::viewer {

ConfigWindow::ConfigWindow(wxWindow* parent, wxWindowID id)
  : wxNotebook(parent, id) {
  CreateImageList();
  SetImages(notebook_images_);

  tree_ = new wxTreeCtrl(this, wxID_ANY);
  tree_->SetImages(tree_images_);

  signal_window_ = new SignalWindow(this, wxID_ANY);

  wxNotebook::InsertPage(kConfigurationPage, tree_,
    "Configuration", true, kBmpResources);

  wxNotebook::InsertPage(kSignalPage, signal_window_,
    "Signals", false, kBmpVariables);

  tree_->Bind(wxEVT_TREE_SEL_CHANGED, &ConfigWindow::OnItemChanged,
    this);
  tree_->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &ConfigWindow::OnRightClick,
    this);
  Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &ConfigWindow::OnNotebookPageChanged,
    this);
}

MdfDocument* ConfigWindow::GetDoc() const {
  wxWindow* parent = GetParent();
  if (parent == nullptr) {
    return nullptr;
  }
  auto* panel = parent->GetParent();
  if (panel == nullptr) {
    return nullptr;
  }
  if (auto* frame =  dynamic_cast<ChildFrame*>(panel->GetParent());
    frame != nullptr) {
    return frame->GetDoc();
  }
  return nullptr;
}

ChildFrame* ConfigWindow::GetChildFrame() const {
  wxWindow* parent = GetParent();
  if (parent == nullptr) {
    return nullptr;
  }
  auto* panel = parent->GetParent();
  if (panel == nullptr) {
    return nullptr;
  }
  return dynamic_cast<ChildFrame*>(panel->GetParent());
}
void ConfigWindow::Redraw() {
      RedrawTree();
      if (signal_window_ != nullptr) {
        signal_window_->Redraw();
      }
}

void ConfigWindow::RedrawTree() const {
  if (tree_ == nullptr) {
    return;
  }
  const wxTreeItemId selected = tree_->GetSelection();
  ConfigData* selected_data = nullptr;
  if (selected.IsOk()) {
    selected_data = dynamic_cast<ConfigData*>(tree_->GetItemData(selected));
  }
  tree_->DeleteAllItems();

  const auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto& config = doc->GetConfiguration();

  const wxString name = config.GetName().empty() ? wxString("Configuration")
      : wxString::FromUTF8(config.GetName());
  auto* root_data = new ConfigData(ConfigPosition::kConfig);
  wxTreeItemId root = tree_->AddRoot(name,
    kBmpConfig, kBmpConfig, root_data);
  RedrawLayers(root);
  RedrawDataFiles(root);
  RedrawDbFiles(root);
  RedrawHexFiles(root);
  RedrawLabFiles(root);
  RedrawAnyFiles(root);
  tree_->Expand(root);

  if (selected_data != nullptr && *selected_data == *root_data) {
    tree_->SelectItem(root);
  } else if (selected_data == nullptr) {
    tree_->SelectItem(root);
  }
}
void ConfigWindow::SelectBlock(const MdfReader* reader,
                               int64_t block_index) const {
  if (tree_ == nullptr || reader == nullptr) {
    return;
  }
  wxTreeItemId item = FindBlock(reader, block_index, tree_->GetRootItem());
  if (item.IsOk()) {
    tree_->SelectItem(item);
    tree_->EnsureVisible(item);
  }
}

int64_t ConfigWindow::GetBlockIndex(const wxTreeItemId& item,
                                     const MdfReader* reader) const {
  if (!item.IsOk() || tree_ == nullptr || reader == nullptr) {
    return -1;
  }
  const auto* data = dynamic_cast<const ConfigData*>(tree_->GetItemData(item));
  if (data == nullptr || data->Reader() != reader) {
    return -1;
  }
  return data->BlockIndex();
}

wxTreeItemId ConfigWindow::FindBlock(const MdfReader* reader,
                                     int64_t block_index,
                                     const wxTreeItemId& root) const {
    if (tree_ == nullptr) {
    return {};
  }
  for (wxTreeItemId item = root; item.IsOk();
       item = tree_->GetNextSibling(item)) {
    if (GetBlockIndex(item, reader) == block_index) {
      return item;
    }
    if (tree_->ItemHasChildren(item)) {
      wxTreeItemIdValue cookie;
      wxTreeItemId find = FindBlock( reader, block_index,
          tree_->GetFirstChild(item, cookie));
      if (find.IsOk()) {
        return find;
      }
    }
  }
  return {};
}

void ConfigWindow::RedrawLayers(const wxTreeItemId& root) const {
  const auto* doc = GetDoc();
  if (doc == nullptr || tree_ == nullptr) {
    return;
  }
  const auto& config = doc->GetConfiguration();
  auto* layers_data = new ConfigData(ConfigPosition::kLayers);
  const wxTreeItemId layers_root = tree_->AppendItem(root,"Layers",
    kBmpLayers, kBmpLayers, layers_data);
  int layer_index = 0;
  for (const Layer& layer : config.GetLayers()) {
    const std::string& name = layer.GetName();
    const wxString layer_name = name.empty() ?
         wxString("Layer" + std::to_string(layer_index + 1))
        : wxString::FromUTF8(name);
    auto* layer_data = new ConfigData(ConfigPosition::kLayer, layer_index);
    tree_->AppendItem(layers_root, layer_name,
      kBmpLinePlot, kBmpLinePlot, layer_data);
    ++layer_index;
  }
  tree_->Expand(layers_root);
}

void ConfigWindow::RedrawDataFiles(const wxTreeItemId& root) const {
  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  auto* folder_data = new ConfigData(ConfigPosition::kDataFiles);
  const wxTreeItemId files_root = tree_->AppendItem(root,"Data Files",
    kBmpFolderOpen, kBmpFolderOpen, folder_data);

  ReaderList& readers = doc->GetReaderList();
  for (size_t index = 0; index < readers.size(); ++index) {
    MdfReader* reader = readers[index].get();
    if (reader == nullptr) {
      continue;
    }
    const std::wstring& file = reader->WFilename();
    try {
      const path fullname(file);
      const std::wstring filename = fullname.filename().wstring();
      const std::wstring parent_path = fullname.parent_path().wstring();
      auto* file_data = new ConfigData(ConfigPosition::kDataFile, index);
      std::wstringstream label;
      label << filename << " (" << parent_path << ")";
      const wxTreeItemId file_root = tree_->AppendItem(files_root,
        label.str(),
        kBmpMf4, kBmpMf4, file_data);

      const MdfFile* mdf_file = reader->GetFile();
      if (const auto* file3 = dynamic_cast<const Mdf3File*>(mdf_file);
          file3 != nullptr ) {
        RedrawMdf3Blocks(file_root, *file3, reader);
      } else if (const auto* file4 = dynamic_cast<const Mdf4File*>(mdf_file);
          file4 != nullptr) {
        RedrawMdf4Blocks(file_root, *file4, reader);
      }
    } catch (const std::exception& e) {
      MDF_ERROR() << "Error processing DATA file: " << e.what();
    }
  }
  tree_->Expand(files_root);
}

void ConfigWindow::RedrawDbFiles(const wxTreeItemId& root) const {
  const auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto& config = doc->GetConfiguration();
  auto* folder_data = new ConfigData(ConfigPosition::kDbFiles);
  const wxTreeItemId files_root = tree_->AppendItem(root,"Database Files",
    kBmpFolderOpen, kBmpFolderOpen, folder_data);

  const FileList& file_list = config.GetDbFiles();
  for (size_t index = 0; index < file_list.size(); ++index) {
    const std::wstring& file = file_list[index];
    try {
      const path fullname(file);
      const std::wstring filename = fullname.filename().wstring();
      const std::wstring parent_path = fullname.parent_path().wstring();
      auto* file_data = new ConfigData(ConfigPosition::kDbFile, index);
      std::wstringstream label;
      label << filename << " (" << parent_path << ")";
      tree_->AppendItem(files_root, label.str(),
      kBmpDatabase, kBmpDatabase, file_data);
    } catch (const std::exception& e) {
      MDF_ERROR() << "Error processing DB file: " << e.what();
    }
  }
  tree_->Expand(files_root);
}

void ConfigWindow::RedrawHexFiles(const wxTreeItemId& root) const {
  const auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto& config = doc->GetConfiguration();
  auto* folder_data = new ConfigData(ConfigPosition::kHexFiles);
  const wxTreeItemId files_root = tree_->AppendItem(root,"Binary Files",
    kBmpFolderOpen, kBmpFolderOpen, folder_data);

  const FileList& file_list = config.GetHexFiles();
  for (size_t index = 0; index < file_list.size(); ++index) {
    const std::wstring& file = file_list[index];
    try {
      const path fullname(file);
      const std::wstring filename = fullname.filename().wstring();
      const std::wstring parent_path = fullname.parent_path().wstring();
      auto* file_data = new ConfigData(ConfigPosition::kHexFile, index);
      std::wstringstream label;
      label << filename << " (" << parent_path << ")";
      tree_->AppendItem(files_root, label.str(),
      kBmpMemory, kBmpMemory, file_data);
    } catch (const std::exception& e) {
      MDF_ERROR() << "Error processing HEX file: " << e.what();
    }
  }
  tree_->Expand(files_root);
}

void ConfigWindow::RedrawLabFiles(const wxTreeItemId& root) const {
  const auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto& config = doc->GetConfiguration();
  auto* folder_data = new ConfigData(ConfigPosition::kLabFiles);
  const wxTreeItemId files_root = tree_->AppendItem(root,"Selection Lists",
    kBmpFolderOpen, kBmpFolderOpen, folder_data);
  const FileList& file_list = config.GetHexFiles();
  for (size_t index = 0; index < file_list.size(); ++index) {
    const std::wstring& file = file_list[index];
    try {
      const path fullname(file);
      const std::wstring filename = fullname.filename().wstring();
      const std::wstring parent_path = fullname.parent_path().wstring();
      auto* file_data = new ConfigData(ConfigPosition::kLabFile, index);
      std::wstringstream label;
      label << filename << " (" << parent_path << ")";
      tree_->AppendItem(files_root, label.str(),
      kBmpSettings, kBmpSettings, file_data);
    } catch (const std::exception& e) {
      MDF_ERROR() << "Error processing LAB file: " << e.what();
    }
  }
  tree_->Expand(files_root);
}

void ConfigWindow::RedrawAnyFiles(const wxTreeItemId& root) const {
  const auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto& config = doc->GetConfiguration();
  auto* folder_data = new ConfigData(ConfigPosition::kAnyFiles);
  const wxTreeItemId files_root = tree_->AppendItem(root,"Other Files",
    kBmpFolderOpen, kBmpFolderOpen, folder_data);
  const FileList& file_list = config.GetHexFiles();
  for (size_t index = 0; index < file_list.size(); ++index) {
    const std::wstring& file = file_list[index];
    try {
      const path fullname(file);
      const std::wstring filename = fullname.filename().wstring();
      const std::wstring parent_path = fullname.parent_path().wstring();
      auto* file_data = new ConfigData(ConfigPosition::kAnyFile, index);
      std::wstringstream label;
      label << filename << " (" << parent_path << ")";
      tree_->AppendItem(files_root, label.str(),
      kBmpDocText, kBmpDocText, file_data);
    } catch (const std::exception& e) {
      MDF_ERROR() << "Error processing ANY file: " << e.what();
    }
  }
  tree_->Expand(files_root);
}

void ConfigWindow::RedrawMdf3Blocks(const wxTreeItemId& root,
                                    const Mdf3File& file,
                                    MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  // ID block
  const IdBlock& id = file.Id();
  std::ostringstream id_string;
  id_string << "ID (" << id.VersionString() << ")";
  tree_->AppendItem(root, wxString::FromUTF8(id_string.str()),
                    kBmpIdBlock, kBmpIdBlock,
                    new ConfigData(reader, id.Index()));

  // Header block
  const Hd3Block& hd3 = file.Hd();
  const auto& hd3_string = CreateBlockText(hd3);
  const wxTreeItemId hd_root = tree_->AppendItem(root,
    wxString::FromUTF8(hd3_string),kBmpHdBlock, kBmpHdBlock,
    new ConfigData(reader, hd3.Index()));
  tree_->Expand(root);
  RedrawMeasurement(hd_root, hd3, reader);

}

void ConfigWindow::RedrawMdf4Blocks(const wxTreeItemId& root,
                                    const Mdf4File& file,
                                    MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }

  // File identity block
  const IdBlock& id = file.Id();
  std::ostringstream id_string;
  id_string << "ID (" << id.VersionString() << ")";
  tree_->AppendItem(root, wxString::FromUTF8(id_string.str()),
                    kBmpIdBlock, kBmpIdBlock,
                    new ConfigData(reader, id.Index()));

  // Header block
  const Hd4Block& hd4 = file.Hd();
  const auto& hd_string = CreateBlockText(hd4);
  const wxTreeItemId hd_root = tree_->AppendItem(root,
    wxString::FromUTF8(hd_string),
        kBmpHdBlock, kBmpHdBlock,
        new ConfigData(reader,hd4.FilePosition()));
  tree_->Expand(root);
  RedrawHistory(hd_root, hd4, reader);
  RedrawMeasurement(hd_root,hd4, reader);
  RedrawHierarchy(hd_root, hd4, reader);
  RedrawAttachment(hd_root, hd4, reader);
  RedrawEvent(hd_root, hd4, reader);
}

void ConfigWindow::RedrawHistory(const wxTreeItemId &root,
                    const Hd4Block &hd4, MdfReader* reader ) const {
  if (hd4.Fh4().empty() || tree_ == nullptr) {
    return;
  }
  const wxTreeItemId fh_root = tree_->AppendItem(root, "Histories",
    kBmpFhRoot,kBmpFhRoot,
    new ConfigData(ConfigPosition::kHistories, reader));
  const auto& fh4_list = hd4.Fh4();
  for (size_t index = 0; index < fh4_list.size(); ++index) {
    const auto& fh4 = fh4_list[index];
    if (!fh4) {
      continue;
    }
    tree_->AppendItem(fh_root, CreateBlockText(*fh4),
                kBmpFhBlock, kBmpFhBlock,
                new ConfigData(reader, fh4->FilePosition()));
  }
}

void ConfigWindow::RedrawMeasurement( const wxTreeItemId &root,
    const Hd4Block &hd4, MdfReader* reader) const {
  const auto& dg_list = hd4.Dg4();
  if (dg_list.empty() || tree_ == nullptr) {
    return;
  }
  const wxTreeItemId dg_root = tree_->AppendItem(root, "Measurements",
    kBmpDgRoot, kBmpDgRoot,
    new ConfigData(ConfigPosition::kMeasurements, reader));

  for (size_t index = 0; index < dg_list.size(); ++index) {
    const auto& dg4 = dg_list[index];
    if (!dg4) {
      continue;
    }
    RedrawDgBlock(dg_root, *dg4, reader);
  }
}


void ConfigWindow::RedrawMeasurement(const wxTreeItemId &root,
              const Hd3Block &hd3, MdfReader* reader) const {
  const auto& dg_list = hd3.Dg3();
  if (dg_list.empty() || tree_ == nullptr) {
    return;
  }
  const wxTreeItemId dg_root = tree_->AppendItem(root, "Measurements",
    kBmpDgRoot, kBmpDgRoot,
    new ConfigData(ConfigPosition::kMeasurements, reader));

  for (size_t index = 0; index < dg_list.size(); ++index) {
    const auto& dg3 = dg_list[index];
    if (!dg3) {
      continue;
    }
    RedrawDgBlock(dg_root, *dg3, reader);
  }
}

void ConfigWindow::RedrawHierarchy(const wxTreeItemId &root, const Hd4Block &hd4,
    MdfReader* reader) const {
  const auto& ch_list = hd4.Ch4();
  if (ch_list.empty() || tree_ == nullptr) {
    return;
  }
  const wxTreeItemId ch_root = tree_->AppendItem(root, "Channel Hierarchies",
    kBmpChRoot, kBmpChRoot,
    new ConfigData(ConfigPosition::kHierarchies, reader));
  for (const auto& ch4 : ch_list) {
    if (!ch4) {
      continue;
    }
    RedrawChBlock(ch_root, *ch4, reader);
  }
}

void ConfigWindow::RedrawChBlock( const wxTreeItemId& root, const Ch4Block &ch,
    MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  std::wostringstream sub_string;
  const wxString ch_name = wxString::FromUTF8(ch.Name());
  const wxString ch_desc = wxString::FromUTF8(ch.Comment());

  if (!ch_name.empty()) {
    sub_string << ch_name;
  }
  if (!ch_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << ch_desc;
    } else {
      sub_string << L":" << ch_desc;
    }
  }

  std::wostringstream ch_string;
  ch_string << wxString::FromUTF8(ch.BlockType());
  if (!sub_string.str().empty() ) {
    ch_string << L" (" << sub_string.str() << L")";
  }

  const wxTreeItemId ch_root = tree_->AppendItem(root, ch_string.str(),
     kBmpChBlock, kBmpChBlock,
     new ConfigData(reader, ch.FilePosition()));
  for (const auto& p : ch.Ch()) {
    if (!p) {
      continue;
    }
    RedrawChBlock(ch_root, *p, reader);
  }
}

void ConfigWindow::RedrawAttachment(const wxTreeItemId& root, const Hd4Block &hd4,
    MdfReader* reader) const {
  const auto& at_list = hd4.At4();
  if (at_list.empty() || tree_ == nullptr) {
    return;
  }
  const wxTreeItemId at_root = tree_->AppendItem(root, "Attachments",
    kBmpAtRoot, kBmpAtRoot,
    new ConfigData(ConfigPosition::kAttachments, reader));
  for (const auto &at4: at_list) {
    if (!at4) {
      continue;
    }

    wxString filename = wxString::FromUTF8(at4->FileName());
    if (filename.empty()) {
      filename = wxString::FromUTF8(CreateBlockText(*at4));
    }
    std::wostringstream block_string;
    block_string << wxString::FromUTF8(at4->BlockType());
    if (!filename.empty()) {
      block_string << " (" << filename << ")";
    }
    tree_->AppendItem(at_root, block_string.str(),
                      kBmpAtBlock, kBmpAtBlock,
                      new ConfigData( reader, at4->FilePosition()));
  }
}

void ConfigWindow::RedrawEvent( const wxTreeItemId& root, const Hd4Block &hd4,
  MdfReader* reader) const {
  const auto& ev_list = hd4.Ev4();
  if (ev_list.empty() || tree_ == nullptr) {
    return;
  }
  const wxTreeItemId ev_root = tree_->AppendItem(root, "Events",
    kBmpEvRoot, kBmpEvRoot,
    new ConfigData(ConfigPosition::kEvents, reader));

  for (const auto& ev4 : ev_list) {
    if (!ev4) {
      continue;
    }
    tree_->AppendItem(ev_root, CreateBlockText(*ev4),
      kBmpEvBlock, kBmpEvBlock,
      new ConfigData(reader, ev4->FilePosition()));
  }
}

void ConfigWindow::RedrawDgBlock(const wxTreeItemId& root,  const Dg4Block &dg4,
      MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  std::wostringstream label;
  label << wxString::FromUTF8(dg4.BlockType());

  const std::string desc = dg4.Description();

  if (!desc.empty()) {
    label << L" (" << wxString::FromUTF8(desc) << L")";
  } else if (dg4.Cg4().size() == 1) {
    const auto* cg4 = dg4.Cg4().front().get();
    if (cg4 != nullptr) {
      std::wostringstream sub;
      sub << wxString::FromUTF8(cg4->Name());
      sub << L" Samples: " << cg4->NofSamples();
      label << L" (" << sub.str() << L")";
    }
  }

  const wxTreeItemId root_dg = tree_->AppendItem(root,
    label.str(),kBmpDgBlock, kBmpDgBlock,
    new ConfigData(reader, dg4.FilePosition()));
  RedrawDataList(root_dg, dg4, reader);
  RedrawCgList(root_dg, dg4, reader);
}

void ConfigWindow::RedrawDgBlock(const wxTreeItemId& root, const Dg3Block &dg3,
             MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  const wxTreeItemId root_dg = tree_->AppendItem(root, CreateBlockText(dg3),
     kBmpDgBlock, kBmpDgBlock,
     new ConfigData(reader, dg3.FilePosition()));
  RedrawDataList(root_dg, dg3, reader);
  RedrawCgList(root_dg, dg3, reader);

  if (const auto* tr3 = dg3.Tr3(); tr3 != nullptr) {
    tree_->AppendItem(root_dg, CreateBlockText(*tr3),
        kBmpEvBlock, kBmpEvBlock,
        new ConfigData(reader, tr3->FilePosition()));
  }
}

void ConfigWindow::RedrawCgList(const wxTreeItemId& root, const Dg4Block& dg4,
        MdfReader* reader) const {
  const auto& cg_list = dg4.Cg4();
  if (cg_list.empty() || tree_ == nullptr) {
    return;
  }

  for (const auto& cg4 : cg_list) {
    if (!cg4) {
      continue;
    }

    std::wostringstream sub_string;
    if (!cg4->Comment().empty()) {
      sub_string << wxString::FromUTF8(cg4->Comment());
    }
    if (!cg4->Name().empty()) {
      if (!sub_string.str().empty()) {
        sub_string << L"/";
      }
      sub_string << wxString::FromUTF8(cg4->Name());
    }
    if (!sub_string.str().empty()) {
      sub_string << L" ";
    }
    sub_string << L"Samples: " << cg4->NofSamples();

    std::wostringstream cg_string;
    cg_string << wxString::FromUTF8(cg4->BlockType()) << L" (" << sub_string.str() << L")";

    const wxTreeItemId root_cg = tree_->AppendItem(root,
      cg_string.str(),
      kBmpCgBlock, kBmpCgBlock,
      new ConfigData(reader, cg4->FilePosition()));

    if (const auto* si = cg4->Source(); si != nullptr) {
      RedrawSiBlock(root_cg, *si, reader);
    }
    RedrawCnList(root_cg,*cg4, reader);
    RedrawSrList(root_cg,*cg4, reader);
  }
}

void ConfigWindow::RedrawCgList(const wxTreeItemId& root, const Dg3Block& dg3,
  MdfReader* reader) const {
  const auto& cg_list = dg3.Cg3();
  if (cg_list.empty() || tree_ == nullptr) {
    return;
  }

  for (const auto& cg3 : dg3.Cg3()) {
    if (!cg3) {
      continue;
    }

    std::ostringstream sub_string;
    if (!cg3->Comment().empty()) {
      sub_string << cg3->Comment();
    }
    if (!cg3->Name().empty()) {
      if (!sub_string.str().empty()) {
        sub_string << "/";
      }
      sub_string << cg3->Name();
    }
    if (!sub_string.str().empty()) {
      sub_string << " ";
    }
    sub_string << "Samples: " << cg3->NofSamples();

    std::ostringstream cg_string;
    cg_string << cg3->BlockType() << " (" << sub_string.str() << ")";

    auto root_cg = tree_->AppendItem(root,
      wxString::FromUTF8(cg_string.str()),
      kBmpCgBlock, kBmpCgBlock,
      new ConfigData(reader, cg3->FilePosition()));
    RedrawCnList(root_cg, *cg3, reader);
    RedrawSrList(root_cg, *cg3, reader);
  }
}

void ConfigWindow::RedrawSiBlock(const wxTreeItemId& root,
                                 const Si4Block& si4,
                                 MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  std::wostringstream sub_string;
  const auto& si_name = si4.Name();
  const auto& si_path = si4.Path();
  const auto si_desc = si4.Comment();

  if (!si_name.empty()) {
    sub_string << wxString::FromUTF8(si_name);
  }

  if (!si_path.empty()) {
    if (sub_string.str().empty()) {
      sub_string << wxString::FromUTF8(si_path);
    } else {
      sub_string << L":" << wxString::FromUTF8(si_path);
    }
  }

  if (!si_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << wxString::FromUTF8(si_desc);
    } else {
      sub_string << L" " << wxString::FromUTF8(si_desc);
    }
  }

  std::wostringstream si_string;
  si_string << wxString::FromUTF8(si4.BlockType()) << L" ("
    << sub_string.str() << L")";

  tree_->AppendItem(root, si_string.str(),
    kBmpSiBlock, kBmpSiBlock,
    new ConfigData(reader, si4.FilePosition()));
}

void ConfigWindow::RedrawCnList(const wxTreeItemId& root, const Cg4Block &cg4,
  MdfReader* reader) const {
  const auto& cn_list = cg4.Cn4();
  if (cn_list.empty() || tree_ == nullptr) {
    return;
  }

  for (const auto& cn4 : cn_list) {
    if (!cn4) {
      continue;
    }
    RedrawCnBlock(root, *cn4, reader);
  }
}

void ConfigWindow::RedrawCnList(const wxTreeItemId& root, const Cg3Block &cg3,
  MdfReader* reader) const {
  const auto& cn_list = cg3.Cn3();
  if (cn_list.empty() || tree_ == nullptr) {
    return;
  }
  for (const auto& cn3 : cn_list) {
    if (!cn3) {
      continue;
    }

    std::wostringstream sub_string;
    std::string cn_name = cn3->Name();
    std::string cn_unit = cn3->Unit();

    sub_string << wxString::FromUTF8(cn_name);
    if (!cn_unit.empty()) {
      sub_string << L" [" << wxString::FromUTF8(cn_unit) << L"]";
    }

    std::wostringstream cn_string;
    cn_string << wxString::FromUTF8(cn3->BlockType()) <<
      L" (" << sub_string.str() << L")";

    const wxTreeItemId cn_root = tree_->AppendItem(root, cn_string.str(),
       kBmpCnBlock, kBmpCnBlock,
       new ConfigData(reader, cn3->FilePosition()));

    if ( const auto* cc3 = cn3->Cc3(); cc3 != nullptr) {
      RedrawCcBlock(cn_root, *cc3, reader);
    }

    if (const auto* cd3 = cn3->Cd3(); cd3 != nullptr) {
      tree_->AppendItem(cn_root, CreateBlockText(*cd3),
      kBmpChBlock, kBmpChBlock,
        new ConfigData(reader, cd3->FilePosition()));
    }

    if (const auto* ce3 = cn3->Ce3(); ce3 != nullptr) {
      tree_->AppendItem(cn_root, CreateBlockText(*ce3),
      kBmpSiBlock, kBmpSiBlock,
      new ConfigData(reader, ce3->FilePosition()));
    }
    RedrawDataList(cn_root, *cn3, reader);
  }
}

void ConfigWindow::RedrawSrList(const wxTreeItemId& root, const Cg4Block &cg4,
  MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }

  for (const auto& sr4 : cg4.Sr4()) {
    if (!sr4) {
      continue;
    }
    std::wostringstream sub_string;
    sub_string << L"Samples: " << sr4->NofSamples()
             << L", Interval: " << sr4->Interval();

    std::wostringstream sr_string;
    sr_string << wxString::FromUTF8(sr4->BlockType())
      << L" (" << sub_string.str() << L")";
    const wxTreeItemId sr_root = tree_->AppendItem(root,sr_string.str(),
        kBmpSrBlock, kBmpSrBlock,
        new ConfigData(reader, sr4->FilePosition()));

    RedrawDataList(sr_root, *sr4, reader);
  }
}

void ConfigWindow::RedrawSrList(const wxTreeItemId& root, const Cg3Block &cg3,
  MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  for (const auto& sr : cg3.Sr3()) {
    if (!sr) {
      continue;
    }
    const wxTreeItemId sr_root = tree_->AppendItem(root, sr->BlockType(),
        kBmpSrBlock, kBmpSrBlock,
        new ConfigData(reader, sr->FilePosition()));

    RedrawDataList(sr_root, *sr, reader);
  }
}

void ConfigWindow::RedrawCnBlock(const wxTreeItemId &root, const Cn4Block &cn4,
     MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  std::wostringstream sub_string;
  const std::string cn_name = cn4.Name();
  const std::string cn_unit = cn4.Unit();

  sub_string << wxString::FromUTF8(cn_name);
  if (!cn_unit.empty()) {
    sub_string << L" [" << wxString::FromUTF8(cn_unit) << L"]";
  }

  std::wostringstream cn_string;
  cn_string << wxString::FromUTF8(cn4.BlockType())
    << L" (" << sub_string.str() << L")";


  for (size_t array = 0; cn4.ChannelArray(array) != nullptr ; ++array) {
    if (const auto* channel_array = cn4.ChannelArray(array);
       channel_array != nullptr) {
      cn_string << L" " << wxString::FromUTF8(channel_array->DimensionAsString());
    }
  }

  const wxTreeItemId cn_root = tree_->AppendItem(root, cn_string.str(),
     kBmpCnBlock, kBmpCnBlock,
     new ConfigData(reader, cn4.FilePosition()));
  if (const auto* si4 = cn4.Si(); si4 != nullptr) {
    RedrawSiBlock(cn_root, *si4, reader);
  }
  if (const auto* cc4 = cn4.Cc();  cc4 != nullptr) {
    RedrawCcBlock(cn_root, *cc4, reader);
  }
  RedrawCxList(cn_root, cn4.Cx4(), reader);
  RedrawDataList(cn_root, cn4, reader);
}

void ConfigWindow::RedrawCcBlock(const wxTreeItemId& root, const Cc4Block &cc4,
    MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  std::wostringstream sub_string;
  const std::string cc_name = cc4.Name();
  const std::string cc_unit = cc4.Unit();
  const std::string cc_desc = cc4.Comment();

  if (!cc_name.empty()) {
    sub_string << wxString::FromUTF8(cc_name);
  }
  if (!cc_unit.empty()) {
    if (sub_string.str().empty()) {
      sub_string << L"[" << wxString::FromUTF8(cc_unit) << L"]";
    } else {
      sub_string << L" [" << wxString::FromUTF8(cc_unit) << L"]";
    }
  }
  if (!cc_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << wxString::FromUTF8(cc_desc);
    } else {
      sub_string << L" " << wxString::FromUTF8(cc_desc);
    }
  }

  std::wostringstream cc_string;
  cc_string << wxString::FromUTF8(cc4.BlockType());
  if (!sub_string.str().empty() ) {
    cc_string << L" (" << sub_string.str() << L")";
  }

  const wxTreeItemId cc_root = tree_->AppendItem(root, cc_string.str(),
     kBmpCcBlock, kBmpCcBlock,
     new ConfigData(reader, cc4.FilePosition()));

  if ( const Cc4Block* reverse = cc4.Cc(); reverse != nullptr) {
    RedrawCcBlock(cc_root, *reverse, reader);
  }
  const auto& ref_list = cc4.References();
  for (const auto& ref : ref_list) {
    if (!ref || ref->BlockType() != "CC") {
      continue;
    }
    const auto* ref4 = dynamic_cast<const mdf::detail::Cc4Block*>(ref.get());
    if (ref4 != nullptr) {
      RedrawCcBlock(cc_root, *ref4, reader);
    }
  }
}

void ConfigWindow::RedrawCcBlock(const wxTreeItemId& root, const Cc3Block &cc3,
    MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }

  std::wostringstream sub_string;
  const wxString cc_name = wxString::FromUTF8(cc3.Name());
  const wxString cc_unit = wxString::FromUTF8(cc3.Unit());
  const wxString cc_desc = wxString::FromUTF8(cc3.Comment());

  if (!cc_name.empty()) {
    sub_string << cc_name;
  }
  if (!cc_unit.empty()) {
    if (sub_string.str().empty()) {
      sub_string << L"[" << cc_unit << L"]";
    } else {
      sub_string << L" [" << cc_unit << L"]";
    }
  }
  if (!cc_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << cc_desc;
    } else {
      sub_string << L" " << cc_desc;
    }
  }

  std::wostringstream cc_string;
  cc_string << wxString::FromUTF8(cc3.BlockType());
  if (!sub_string.str().empty() ) {
    cc_string << L" (" << sub_string.str() << L")";
  }

  tree_->AppendItem(root, cc_string.str(),
      kBmpCcBlock, kBmpCcBlock,
      new ConfigData(reader, cc3.FilePosition()));
}

void ConfigWindow::RedrawCxList(const wxTreeItemId& root,
  const std::vector<std::unique_ptr<MdfBlock>>& cx_list,
  MdfReader* reader) const {
  if (cx_list.empty() || tree_ == nullptr) {
    return;
  }

  for (const auto& cx4 : cx_list) {
    if (!cx4) {
      continue;
    }
    if (cx4->BlockType() == "CA") {
      const auto* ca_block = dynamic_cast<const Ca4Block*>(cx4.get());
      if (ca_block == nullptr) {
        continue;
      }
      RedrawCaBlock(root, *ca_block, reader);
    } else if (cx4->BlockType() == "CN") {
      const auto* cn_block = dynamic_cast<const Cn4Block*>(cx4.get());
      if (cn_block == nullptr) {
        continue;
      }
      RedrawCnBlock(root, *cn_block, reader);
    }
  }
}

void ConfigWindow::RedrawCaBlock(const wxTreeItemId &root, const Ca4Block &ca4,
  MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  const wxString type = wxString::FromUTF8(ca4.TypeAsString());
  const wxString storage = wxString::FromUTF8(ca4.StorageAsString());

  std::wostringstream sub_string;
  sub_string << type << L"/" << storage;


  std::wostringstream label;
  label << wxString::FromUTF8(ca4.BlockType())
    << L" (" << sub_string.str() << L") "
    << wxString::FromUTF8(ca4.DimensionAsString());

  tree_->AppendItem(root, label.str(),
    kBmpCaBlock, kBmpCaBlock,
    new ConfigData(reader, ca4.FilePosition()));
}
void ConfigWindow::RedrawDataList(const wxTreeItemId&root,
    const DataListBlock& data_list,  MdfReader* reader) const {
  if (tree_ == nullptr) {
    return;
  }
  for (const auto& data : data_list.DataBlockList()) {
    if (!data) {
      continue;
    }
    std::wostringstream block_string;
    block_string << wxString(data->BlockType());
    const auto* data_block = dynamic_cast<const DataBlock*>(data.get());
    const auto* data_block_list = dynamic_cast<const DataListBlock*>(data.get());
    uint64_t nof_data_bytes = data_block != nullptr ?
        data_block->DataSize() : 0;
    if (data_block_list != nullptr) {
      nof_data_bytes =data_block_list->DataSize();
    }

    if (data->BlockType() == "DT") {
      block_string << " (Size [byte]: " << nof_data_bytes << ")";
      tree_->AppendItem(root, block_string.str(),
                        kBmpDtBlock, kBmpDtBlock,
                        new ConfigData(reader, data->FilePosition()));
    } else if (data->BlockType() == "DV" || data->BlockType() == "DI") {
      block_string << " (Size [byte]: " << nof_data_bytes << ")";
      tree_->AppendItem(root,block_string.str(),
                        kBmpDtBlock, kBmpDtBlock,
                        new ConfigData(reader, data->FilePosition()));
    } else if (data->BlockType() == "DZ") {
      if (const auto* dz4 = dynamic_cast<const Dz4Block*>(data.get());
          dz4 != nullptr) {
        block_string << "/" << wxString(dz4->OrigBlockType())
          << " (Size [byte]: " << dz4->CompressedDataSize()
          << "/" << dz4->DataSize() << ")";
      }
      tree_->AppendItem(root,block_string.str(),
                        kBmpDzBlock, kBmpDzBlock,
                        new ConfigData(reader, data->FilePosition()));
    } else if (data->BlockType() == "RD" || data->BlockType() == "RV" ||
                data->BlockType() == "RI") {
      block_string << " (Size [byte]: " << nof_data_bytes << ")";
      tree_->AppendItem(root, block_string.str(), kBmpRdBlock, kBmpRdBlock,
                        new ConfigData(reader, data->FilePosition()));
    } else if (data->BlockType() == "SD") {
      block_string << " (Size [byte]: " << nof_data_bytes << ")";
      tree_->AppendItem(root, block_string.str(),
                        kBmpSdBlock, kBmpSdBlock,
                        new ConfigData(reader, data->FilePosition()));
    } else if (data->BlockType() == "DL") {
      if (const auto* dl4 = dynamic_cast<const Dl4Block*>(data.get());
          dl4 != nullptr ) {
        block_string << " (Nof Blocks: " << dl4->DataBlockList().size() << ")";
        const wxTreeItemId dl_root = tree_->AppendItem(root,block_string.str(),
          kBmpDlBlock, kBmpDlBlock,
          new ConfigData(reader, data->FilePosition()));
        RedrawDataList( dl_root, *dl4, reader);
      }
    } else if (data->BlockType() == "LD") {
      const wxTreeItemId ld_root = tree_->AppendItem(root,block_string.str(),
                     kBmpDlBlock, kBmpDlBlock,
                      new ConfigData(reader, data->FilePosition()));
      if (const auto* list = dynamic_cast<const DataListBlock*>(data.get());
          list != nullptr) {
        RedrawDataList(ld_root, *list, reader);
      }
    } else if (data->BlockType() == "HL") {
      if (const auto* hl4 = dynamic_cast<const Hl4Block*>(data.get());
          hl4 != nullptr) {
        block_string << " (" << wxString(hl4->TypeAsString()) << ")";
        const wxTreeItemId hl_root = tree_->AppendItem(root,
          block_string.str(),kBmpHlBlock, kBmpHlBlock,
                        new ConfigData(reader, data->FilePosition()));
        RedrawDataList(hl_root, *hl4, reader);
      }
    } else if (data->BlockType() == "SR") {
      if (const auto* data_list_block =  dynamic_cast<const DataListBlock*>(data.get());
        data_list_block != nullptr) {
        block_string << "(Size [byte]: " << nof_data_bytes << ")";
        const wxTreeItemId sr_root = tree_->AppendItem(root,
          block_string.str(), kBmpSrBlock, kBmpSrBlock,
          new ConfigData(reader, data->FilePosition()));
        RedrawDataList(sr_root, *data_list_block, reader);
      }
    }
  }
}

void ConfigWindow::OnItemChanged(wxTreeEvent& event) {
  const wxTreeItemId& item = event.GetItem();
  if (!item.IsOk() || tree_ == nullptr) {
    return;
  }

  auto* data = dynamic_cast<ConfigData*>(tree_->GetItemData(item));
  if (data == nullptr) {
    return;
  }
  if (MdfDocument* doc = GetDoc(); doc != nullptr) {
    doc->SetLastConfigData(*data);
  }

  if (ChildFrame* frame = GetChildFrame(); frame != nullptr) {
    frame->RedrawDisplays();
  }
}

void ConfigWindow::OnRightClick(wxTreeEvent& event) {
  const wxTreeItemId item = event.GetItem();
  if (!item.IsOk() || tree_ == nullptr) {
    return;
  }
  tree_->SelectItem(item);
  auto* data = dynamic_cast<ConfigData*>(tree_->GetItemData(item));
  if (data == nullptr) {
    return;
  }
  const ConfigPosition position = data->Position();

  wxMenu menu;

  switch (position) {
    case ConfigPosition::kLayers: {
      auto* submenu = new wxMenu;
      submenu->Append(kIdAddLineplot,L"Line Plot");
      menu.AppendSubMenu(submenu,L"Add Layer");
      break;
    }

    case ConfigPosition::kLayer: {
      auto* submenu = new wxMenu;
      submenu->Append(kIdAddLineplot,L"Line Plot");
      menu.AppendSubMenu(submenu,L"Add Layer");
      menu.AppendSeparator();
      menu.Append(kIdDeleteLayer,L"Delete Layer");
      break;
    }

    default:
      return;
  }

  PopupMenu(&menu);
}

void ConfigWindow::OnNotebookPageChanged(wxBookCtrlEvent& event) {
  /*
  const int sel = event.GetSelection();
  if (sel == wxNOT_FOUND) {
    return;
  }
  switch (static_cast< size_t>(sel)) {
    case kConfigurationPage:
      RedrawTree();
      break;

    case kSignalPage:
      if (signal_window_ != nullptr) {
        signal_window_->Redraw();
      }
      break;

    default:
      break;
  }
  */
}


void ConfigWindow::CreateImageList() {
  const std::array<wxBitmapBundle, 34> image_list = {
    wxBitmapBundle(wxBITMAP_PNG(config)),
    wxBitmapBundle(wxBITMAP_PNG(layers)),
    wxBitmapBundle(wxBITMAP_PNG(lineplot)),
    wxBitmapBundle(wxBITMAP_PNG(database)),
    wxBitmapBundle(wxBITMAP_PNG(document_text)),

    wxBitmapBundle(wxBITMAP_PNG(folder_close)),
    wxBitmapBundle(wxBITMAP_PNG(folder_open)),
    wxBitmapBundle(wxBITMAP_PNG(memory)),
    wxBitmapBundle(wxBITMAP_PNG(mf4)),
    wxBitmapBundle(wxBITMAP_PNG(settings)),

    wxBitmapBundle(wxBITMAP_PNG(at_block)),
    wxBitmapBundle(wxBITMAP_PNG(at_root)),
    wxBitmapBundle(wxBITMAP_PNG(ca_block)),
    wxBitmapBundle(wxBITMAP_PNG(cc_block)),
    wxBitmapBundle(wxBITMAP_PNG(cg_block)),

    wxBitmapBundle(wxBITMAP_PNG(ch_block)),
    wxBitmapBundle(wxBITMAP_PNG(ch_root)),
    wxBitmapBundle(wxBITMAP_PNG(cn_block)),
    wxBitmapBundle(wxBITMAP_PNG(dg_block)),
    wxBitmapBundle(wxBITMAP_PNG(dg_root)),

    wxBitmapBundle(wxBITMAP_PNG(dl_block)),
    wxBitmapBundle(wxBITMAP_PNG(dt_block)),
    wxBitmapBundle(wxBITMAP_PNG(dz_block)),
    wxBitmapBundle(wxBITMAP_PNG(ev_block)),
    wxBitmapBundle(wxBITMAP_PNG(ev_root)),

    wxBitmapBundle(wxBITMAP_PNG(fh_block)),
    wxBitmapBundle(wxBITMAP_PNG(fh_root)),
    wxBitmapBundle(wxBITMAP_PNG(hd_block)),
    wxBitmapBundle(wxBITMAP_PNG(hl_block)),
    wxBitmapBundle(wxBITMAP_PNG(id_block)),

    wxBitmapBundle(wxBITMAP_PNG(rd_block)),
    wxBitmapBundle(wxBITMAP_PNG(sd_block)),
    wxBitmapBundle(wxBITMAP_PNG(si_block)),
    wxBitmapBundle(wxBITMAP_PNG(sr_block)),
  };

  for (const auto& image : image_list) {
    tree_images_.emplace_back(image);
  }

  const std::vector<wxBitmap> config_list = {
    wxBITMAP_PNG(resources16),
    wxBITMAP_PNG(resources24),
    wxBITMAP_PNG(resources32),
  };
  const std::vector<wxBitmap> signal_list = {
    wxBITMAP_PNG(variables16),
    wxBITMAP_PNG(variables24),
    wxBITMAP_PNG(variables32),
  };
  notebook_images_.emplace_back(wxBitmapBundle::FromBitmaps(config_list));
  notebook_images_.emplace_back(wxBitmapBundle::FromBitmaps(signal_list));
}

} // mdf::viewer