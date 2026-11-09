/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <wx/wx.h>
#include <wx/docmdi.h>
#include <wx/splitter.h>

#include "configwindow.h"
#include "displaywindow.h"

namespace mdf::viewer {

class MdfDocument;

class ChildFrame : public wxDocMDIChildFrame {
 public:
  ChildFrame(wxDocument *doc,
            wxView *view,
            wxMDIParentFrame *parent,
            wxWindowID id,
            const wxString& title);
  ChildFrame() = default;

  void Update() override;
  void RedrawDisplays() const;
  [[nodiscard]] MdfDocument* GetDoc() const;

 private:
  ConfigWindow* left_ = nullptr;
  DisplayWindow* right_ = nullptr;

  wxSplitterWindow* splitter_ = nullptr;
  wxDECLARE_EVENT_TABLE();

};
}




