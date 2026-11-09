/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "signallistview.h"

#include <algorithm>
#include <ranges>

#include "signalwindow.h"
#include "childframe.h"
#include "mdfdocument.h"

#include "windowid.h"

namespace mdf::viewer {
SignalListView::SignalListView(wxWindow* parent, wxWindowID id)
: wxListView(parent, id, wxDefaultPosition, wxDefaultSize,
    wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VIRTUAL) {

  InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 250);
  InsertColumn(1, "Unit",wxLIST_FORMAT_LEFT, 75);
  InsertColumn(2, "Samples",wxLIST_FORMAT_LEFT, 75);

  wxListCtrl::EnableCheckBoxes(true);
  Bind(wxEVT_LIST_ITEM_CHECKED, &SignalListView::OnItemChecked,
    this);
  Bind(wxEVT_LIST_ITEM_UNCHECKED, &SignalListView::OnItemUnchecked,
    this);
  Bind(wxEVT_LIST_ITEM_SELECTED, &SignalListView::OnItemSelected, this);
}

MdfDocument* SignalListView::GetDoc() const {
  const auto* signal_window = dynamic_cast<SignalWindow*>(GetParent());
  return signal_window != nullptr ? signal_window->GetDoc() : nullptr;
}

ChildFrame* SignalListView::GetChildFrame() const {
  const auto* signal_window = dynamic_cast<SignalWindow*>(GetParent());
  return signal_window != nullptr ? signal_window->GetChildFrame() : nullptr;
}

void SignalListView::Redraw() {
  long selected = GetFirstSelected();
  if (selected < 0) {
    selected = 0;
  }

  MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  DeleteAllItems();

  CreateFilterList();

  SetItemCount(static_cast<long>(filtered_list_.size()));
  if (selected >= 0 && selected < filtered_list_.size()) {
    Select(selected);
    EnsureVisible(selected);
  }
}

wxString SignalListView::OnGetItemText(long item, long column) const {
  if (item < 0 ) {
    return {};
  }
  const size_t index = static_cast<size_t>(item);
  if (index >= filtered_list_.size()) {
    return {};
  }
  const ISignal* signal = filtered_list_[index];
  if (signal == nullptr) {
    return {};
  }
  switch (column) {
    case 0:
      return wxString::FromUTF8(signal->Name());

    case 1:
      return wxString::FromUTF8(signal->Unit());

    case 2:
      return std::to_wstring(signal->NofSamples());

    default:
      break;
  }
  return {};
}

bool SignalListView::OnGetItemIsChecked(long item) const {
  if (item < 0 ) {
    return {};
  }
  const size_t index = static_cast<size_t>(item);
  if (index >= filtered_list_.size()) {
    return {};
  }
  const ISignal* signal = filtered_list_[index];
  if (signal == nullptr) {
    return {};
  }
  return signal->IsSelected();
}

void SignalListView::CreateFilterList() {
  filtered_list_.clear();
  const MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const SignalList& signals = doc->GetSignalList();
  for (const auto& signal : signals) {
    if (!signal) {
      continue;
    }

    filtered_list_.push_back(signal.get());
  }

  std::ranges::sort(filtered_list_,
    [](const ISignal* signalA, const ISignal* signalB)->bool {
    if (signalA == nullptr || signalB == nullptr) {
      return false;
    }
    return signalA->Name() < signalB->Name();
  });
}

void SignalListView::OnItemChecked(wxListEvent& event) {
  const long item = event.GetIndex();
  if (item < 0) {
    return;
  }
  const auto index = static_cast<size_t>(item);
  if (index >= filtered_list_.size()) {
    return;
  }

  ISignal* signal = filtered_list_[index];
  if (signal == nullptr) {
    return;
  }
  signal->SetSelected(true);
  RefreshItem(item);
}

void SignalListView::OnItemUnchecked(wxListEvent& event) {
  const long item = event.GetIndex();
  if (item < 0) {
    return;
  }
  const auto index = static_cast<size_t>(item);
  if (index >= filtered_list_.size()) {
    return;
  }

  ISignal* signal = filtered_list_[index];
  if (signal == nullptr) {
    return;
  }
  signal->SetSelected(false);
  RefreshItem(item);
}

void SignalListView::OnItemSelected(wxListEvent& event) {
  const long item = event.GetIndex();
  if (item < 0) {
    return;
  }
  const auto index = static_cast<size_t>(item);
  if (index >= filtered_list_.size()) {
    return;
  }

  ISignal* signal = filtered_list_[index];
  if (signal == nullptr) {
    return;
  }
  MdfDocument* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const ConfigData data(signal->Reader(), signal->BlockIndex());
  doc->SetLastConfigData(data);

  if (ChildFrame* frame = GetChildFrame(); frame != nullptr) {
    frame->RedrawDisplays();
  }
}

} // mdf::viewer