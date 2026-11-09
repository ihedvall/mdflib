/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <wx/docmdi.h>
#include <wx/wx.h>

namespace mdf::viewer {
class MainFrame : public wxDocMDIParentFrame {
 public:
  MainFrame(const wxString& title, const wxPoint& start_pos,
    const wxSize& start_size, bool maximized);
  ~MainFrame() override;
 private:
  wxTimer* timer_ = nullptr;

  void OnClose(wxCloseEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnUpdateNoDoc(wxUpdateUIEvent& event);
  void OnDropFiles(wxDropFilesEvent& event);
  void OnTimer(wxTimerEvent& event);
  void OnAppearance(wxCommandEvent& event);
wxDECLARE_EVENT_TABLE();
};
}

