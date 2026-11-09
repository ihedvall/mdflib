/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/srchctrl.h>

#include "signallistview.h"

namespace mdf::viewer {

class MdfDocument;
class ChildFrame;
class SignalWindow : public wxPanel {
public:
  SignalWindow(wxWindow* parent, wxWindowID id = wxID_ANY);
  MdfDocument* GetDoc() const;
  ChildFrame* GetChildFrame() const;
  void Redraw() const;
private:
  SignalListView* list_ = nullptr;
  wxSearchCtrl* name_ctrl_ = nullptr;
  wxChoice* unit_ctrl_ = nullptr;
  wxChoice* source_ctrl_ = nullptr;
  wxChoice* lab_ctrl_ = nullptr;

  wxString name_filter_ = "*";
  wxString unit_filter_;
  wxString source_filter_;
  wxString lab_filter_;
};

}  // namespace mdf::viewer


