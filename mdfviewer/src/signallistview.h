/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <deque>

#include <wx/listctrl.h>

#include "mdf/isignal.h"

namespace mdf::viewer {

class MdfDocument;
class ChildFrame;

class SignalListView : public wxListView {
public:
  SignalListView(wxWindow* parent, wxWindowID id = wxID_ANY);
  MdfDocument* GetDoc() const;
  [[nodiscard]] ChildFrame* GetChildFrame() const;
  void Redraw();
protected:
  [[nodiscard]] wxString OnGetItemText(long item, long column) const override;
  [[nodiscard]] bool OnGetItemIsChecked (long item) const override;
private:
  std::deque<ISignal*> filtered_list_;
  void CreateFilterList();
  void OnItemChecked(wxListEvent& event);
  void OnItemUnchecked(wxListEvent& event);
  void OnItemSelected(wxListEvent& event);
};

}  // namespace mdf::viewer

