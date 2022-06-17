/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <wx/wx.h>
#include <wx/docmdi.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include "../../mdflib/src/mdf3file.h"
#include "../../mdflib/src/mdf4file.h"

namespace mdf::viewer {
class ChildFrame : public wxDocMDIChildFrame {
 public:
  ChildFrame(wxDocument *doc,
            wxView *view,
            wxMDIParentFrame *parent,
            wxWindowID id,
            const wxString& title);
  ChildFrame() = default;

 void Update() override;
 private:
  wxTreeCtrl* left_ = nullptr;
  wxListView* right_ = nullptr;

  wxSplitterWindow* splitter_ = nullptr;
  wxImageList image_list_;
  mdf::detail::BlockPropertyList property_list_;


  void RedrawTreeList();

  void RedrawMdf3Blocks(const mdf::detail::Mdf3File* file);
  void RedrawMdf4Blocks(const mdf::detail::Mdf4File* file);

  void RedrawHistory(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);

  void RedrawMeasurement(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);
  void RedrawMeasurement(const mdf::detail::Hd3Block& hd, const wxTreeItemId& root);

  void RedrawHierarchy(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);
  void RedrawAttachment(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);
  void RedrawEvent(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);

  void RedrawDataList(const mdf::detail::DataListBlock& dg, const wxTreeItemId& root);

  void RedrawCgList(const mdf::detail::Dg4Block& dg4, const wxTreeItemId& root);
  void RedrawCgList(const mdf::detail::Dg3Block& dg3, const wxTreeItemId& root);

  void RedrawCnList(const mdf::detail::Cg4Block& cg, const wxTreeItemId& root);
  void RedrawCnList(const mdf::detail::Cg3Block& cg3, const wxTreeItemId& root);

  void RedrawSrList(const mdf::detail::Cg4Block& cg, const wxTreeItemId& root);
  void RedrawSrList(const mdf::detail::Cg3Block& cg, const wxTreeItemId& root);

  void RedrawDgBlock(const mdf::detail::Dg4Block& dg4, const wxTreeItemId& root);
  void RedrawDgBlock(const mdf::detail::Dg3Block& dg3, const wxTreeItemId& root);

  void RedrawSiBlock(const mdf::detail::Si4Block& si, const wxTreeItemId& root);

  void RedrawCcBlock(const mdf::detail::Cc4Block& cc, const wxTreeItemId& root);
  void RedrawCcBlock(const mdf::detail::Cc3Block& cc3, const wxTreeItemId& root);

  void RedrawChBlock(const mdf::detail::Ch4Block& ch, const wxTreeItemId& root);

  void RedrawListView();

  void OnTreeSelected(wxTreeEvent& event);
  void OnTreeRightClick(wxTreeEvent& event);
  void OnListItemActivated(wxListEvent& event);
  wxDECLARE_EVENT_TABLE();

};
}




