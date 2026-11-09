/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <wx/docmdi.h>
#include "mdfview.h"
#include "mdfviewer.h"
#include "childframe.h"
#include "mainframe.h"

namespace mdf::viewer {

wxIMPLEMENT_DYNAMIC_CLASS(MdfView,wxView) //NOLINT

MdfDocument *MdfView::GetDoc() const {
  return dynamic_cast<MdfDocument*>(GetDocument());
}

void MdfView::OnDraw(wxDC*) {

}
bool MdfView::OnCreate(wxDocument *doc, long flags) {
  if (!wxView::OnCreate( doc,flags)) {
    return false;
  }

  auto & app = wxGetApp();
  if (auto* parent = dynamic_cast<MainFrame*>(app.GetTopWindow());
    parent != nullptr) {
    auto* sub_frame = new ChildFrame(doc, this, parent,wxID_ANY,"MDF File");
    sub_frame->Maximize(true);
    sub_frame->Show();
    sub_frame->Update();

  }

  return true;
}

bool MdfView::OnClose(bool del) {
  return wxView::OnClose(del);
}
void MdfView::OnUpdate(wxView *sender, wxObject *hint) {
  wxView::OnUpdate(sender, hint);
  auto* frame = GetFrame();
  if (frame != nullptr) {
    frame->Update();
  }
}

}