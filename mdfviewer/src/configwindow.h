/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/bmpbndl.h>
#include <wx/vector.h>

#include "mdfdocument.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "hd4block.h"
#include "hd3block.h"
#include "ca4block.h"
#include "signalwindow.h"

namespace mdf::viewer {
class ChildFrame;
class MdfDocument;


class ConfigWindow : public wxNotebook {
public:
  ConfigWindow(wxWindow* parent, wxWindowID id);
  [[nodiscard]] MdfDocument* GetDoc() const;
  [[nodiscard]] ChildFrame* GetChildFrame() const;
  void Redraw();
  void RedrawTree() const;
  void SelectBlock(const MdfReader* reader, int64_t block_index) const;

private:
  wxTreeCtrl* tree_ = nullptr;
  SignalWindow* signal_window_ = nullptr;
  wxVector<wxBitmapBundle> tree_images_;
  wxVector<wxBitmapBundle> notebook_images_;

  void CreateImageList();
  void RedrawLayers(const wxTreeItemId& root) const;
  void RedrawDataFiles(const wxTreeItemId& root) const;
  void RedrawDbFiles(const wxTreeItemId& root) const;
  void RedrawHexFiles(const wxTreeItemId& root) const;
  void RedrawLabFiles(const wxTreeItemId& root) const;
  void RedrawAnyFiles(const wxTreeItemId& root) const;
  void RedrawMdf3Blocks(const wxTreeItemId& root,
                       const detail::Mdf3File& file,
                       MdfReader* reader) const;
  void RedrawMdf4Blocks(const wxTreeItemId& root,
                       const detail::Mdf4File& file,
                       MdfReader* reader) const;
  void RedrawHistory(const wxTreeItemId &root, const detail::Hd4Block &hd4,
                     MdfReader* reader ) const;
  void RedrawMeasurement(const wxTreeItemId &root, const detail::Hd4Block &hd4,
                     MdfReader* reader ) const;
  void RedrawMeasurement(const wxTreeItemId &root, const detail::Hd3Block &hd3,
                     MdfReader* reader ) const;
  void RedrawHierarchy(const wxTreeItemId &root, const detail::Hd4Block &hd4,
        MdfReader* reader) const;
  void RedrawChBlock( const wxTreeItemId& root, const detail::Ch4Block &ch,
    MdfReader* reader) const;
  void RedrawAttachment(const wxTreeItemId& root, const detail::Hd4Block &hd4,
        MdfReader* reader) const;
  void RedrawEvent( const wxTreeItemId& root, const detail::Hd4Block &hd4,
        MdfReader* reader) const;
  void RedrawDgBlock(const wxTreeItemId& root,
      const detail::Dg4Block &dg4, MdfReader* reader) const;
  void RedrawDgBlock(const wxTreeItemId& root,
      const detail::Dg3Block &dg3, MdfReader* reader) const;
  void RedrawDataList(const wxTreeItemId& root,
      const detail::DataListBlock& data_list, MdfReader* reader) const;
  void RedrawCgList(const wxTreeItemId& root, const detail::Dg4Block& dg4,
        MdfReader* reader) const;
  void RedrawCgList(const wxTreeItemId& root, const detail::Dg3Block& dg3,
        MdfReader* reader) const;
  void RedrawSiBlock(const wxTreeItemId& root, const detail::Si4Block& si4,
        MdfReader* reader) const;
  void RedrawCnList(const wxTreeItemId& root, const detail::Cg4Block &cg4,
        MdfReader* reader) const;
  void RedrawCnList(const wxTreeItemId& root, const detail::Cg3Block &cg3,
        MdfReader* reader) const;
  void RedrawSrList(const wxTreeItemId& root, const detail::Cg4Block &cg4,
        MdfReader* reader) const;
  void RedrawSrList(const wxTreeItemId& root, const detail::Cg3Block &cg3,
        MdfReader* reader) const;
  void RedrawCnBlock(const wxTreeItemId &root, const detail::Cn4Block &cn4,
        MdfReader* reader) const;
  void RedrawCcBlock(const wxTreeItemId& root, const detail::Cc4Block &cc4,
      MdfReader* reader) const;
  void RedrawCcBlock(const wxTreeItemId& root, const detail::Cc3Block &cc4,
      MdfReader* reader) const;
  void RedrawCxList(const wxTreeItemId& root,
      const std::vector<std::unique_ptr<detail::MdfBlock>>& cx_list,
      MdfReader* reader) const;
  void RedrawCaBlock(const wxTreeItemId &root, const detail::Ca4Block &ca4,
      MdfReader* reader) const;

  void OnItemChanged(wxTreeEvent& event);
  void OnRightClick(wxTreeEvent& event);
  void OnNotebookPageChanged(wxNotebookEvent& event);
  int64_t GetBlockIndex(const wxTreeItemId& item,
                        const MdfReader* reader) const;
  wxTreeItemId FindBlock(const MdfReader* reader,
                       int64_t block_index,
                       const wxTreeItemId& root) const;
};

}  // namespace mdf::viewer


