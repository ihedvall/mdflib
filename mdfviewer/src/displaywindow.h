/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <wx/notebook.h>
#include <wx/bmpbndl.h>
#include <wx/vector.h>

namespace mdf::viewer {

class MdfDocument;
class InfoWindow;

class DisplayWindow : public wxNotebook {
public:
  DisplayWindow(wxWindow* parent, wxWindowID id);
  [[nodiscard]] MdfDocument* GetDoc() const;
  void Redraw();
private:
  wxVector<wxBitmapBundle> image_list_;
  InfoWindow* info_window_ = nullptr;
  void CreateImageList();
};

}  // namespace mdf::viewer


