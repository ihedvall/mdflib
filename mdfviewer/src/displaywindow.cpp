/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "displaywindow.h"

#include <array>
#include <wx/bitmap.h>

#include "childframe.h"
#include "infowindow.h"
#include "windowid.h"

namespace {
#include "img/info32.cpp"
#include "img/info24.cpp"
#include "img/info16.cpp"

constexpr int kBmpInfo = 0;

constexpr size_t kInfoPage = 0;

}

namespace mdf::viewer {
DisplayWindow::DisplayWindow(wxWindow* parent, wxWindowID id)
  : wxNotebook(parent, id, wxDefaultPosition, wxDefaultSize,
    wxNB_TOP) {
  CreateImageList();
  SetImages(image_list_);

  info_window_ = new InfoWindow(this, kIdInfoWindow);
  wxNotebook::InsertPage(kInfoPage, info_window_,
    "Information", true, kBmpInfo);

}

MdfDocument* DisplayWindow::GetDoc() const {
  wxWindow* parent = GetParent();
  if (parent == nullptr) {
    return nullptr;
  }
  wxWindow* panel = parent->GetParent();
  if (panel == nullptr) {
    return nullptr;
  }
  if (auto* frame =  dynamic_cast<ChildFrame*>(panel->GetParent());
    frame != nullptr) {
    return frame->GetDoc();
  }
  return nullptr;
}

void DisplayWindow::Redraw() {
  const int selected_page = GetSelection();
  if (selected_page == wxNOT_FOUND) {
    return;
  }

  switch (static_cast<size_t>(selected_page)) {
    case kInfoPage:
      if (info_window_ != nullptr) {
        info_window_->Redraw();
      }
      break;

    default:
      break;
  }
}

void DisplayWindow::CreateImageList() {
  const std::vector<wxBitmap> info_list = {
    wxBITMAP_PNG(info16),
    wxBITMAP_PNG(info24),
    wxBITMAP_PNG(info32),
  };

  image_list_.emplace_back(wxBitmapBundle::FromBitmaps(info_list));

}

} // mdf::viewer