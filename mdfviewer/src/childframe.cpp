/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "childframe.h"

#include <wx/sizer.h>

#include "mdfdocument.h"
#include "windowid.h"


using namespace mdf::detail;

namespace {
#include "img/app.xpm"
}



namespace mdf::viewer {

wxBEGIN_EVENT_TABLE(ChildFrame, wxDocMDIChildFrame) // NOLINT(cert-err58-cpp)
wxEND_EVENT_TABLE()

ChildFrame::ChildFrame(wxDocument *doc,
                     wxView *view,
                     wxMDIParentFrame *parent,
                     wxWindowID id,
                     const wxString& title)
    : wxDocMDIChildFrame(doc, view, parent, id, title,
      wxDefaultPosition, wxDefaultSize,
                        wxDEFAULT_FRAME_STYLE,
                        wxASCII_STR(wxFrameNameStr)) {
  wxIcon app{wxICON(app)};
  SetIcon(app);

  auto* main_panel = new wxPanel(this);

  splitter_ = new wxSplitterWindow(main_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxCLIP_CHILDREN);

  left_ = new ConfigWindow(splitter_,kIdLeftTree);
  right_ = new DisplayWindow(splitter_,kIdRightWindow);
  splitter_->SplitVertically(left_, right_, 450);

  auto* main_sizer = new wxBoxSizer(wxVERTICAL);
  main_sizer->Add(splitter_, 1 , wxALL | wxGROW,0);
  main_panel->SetSizerAndFit(main_sizer);

}

void ChildFrame::Update() {
  static bool recursive_lock = false;
  wxDocMDIChildFrame::Update();
  if (!recursive_lock) {
    recursive_lock = true;
    left_->Redraw();
    right_->Redraw();
    recursive_lock = false;
  }
  if (MdfDocument* doc = GetDoc(); doc != nullptr) {
    SetTitle(doc->GetTitle());
  }
}

void ChildFrame::RedrawDisplays() const {
  if (right_ != nullptr) {
    right_->Redraw();
  }
}

MdfDocument *ChildFrame::GetDoc() const {
  return dynamic_cast<MdfDocument*>(GetDocument());
}
}



