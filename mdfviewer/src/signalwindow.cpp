/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "signalwindow.h"

#include <wx/sizer.h>
#include <wx/valgen.h>
#include <wx/stattext.h>

#include "configwindow.h"
#include "mdfdocument.h"
#include "windowid.h"

namespace mdf::viewer {

SignalWindow::SignalWindow(wxWindow* parent, wxWindowID id)
  : wxPanel(parent, id) {
  list_  = new SignalListView(this, kIdSignalList);
  name_ctrl_ = new wxSearchCtrl(this, kIdNameFilter,
    name_filter_, wxDefaultPosition, wxDefaultSize,
    wxTE_PROCESS_ENTER | wxTE_LEFT);
  name_ctrl_->ShowSearchButton(true);
  name_ctrl_->ShowCancelButton(true);
  name_ctrl_->SetDescriptiveText("Filter Name");
  name_ctrl_->SetMinSize(wxSize(150, -1));

  const std::vector<wxString> unit_list = {L""};
  unit_ctrl_ = new wxChoice(this, kIdUnitFilter,
    wxDefaultPosition, wxDefaultSize, unit_list, wxCB_SORT,
    wxGenericValidator(&unit_filter_));
  unit_ctrl_->SetMinSize(wxSize(75, -1));

  const std::vector<wxString> source_list = {L""};
  source_ctrl_ = new wxChoice(this, kIdSourceFilter,
    wxDefaultPosition, wxDefaultSize, source_list, wxCB_SORT,
    wxGenericValidator(&source_filter_));
  source_ctrl_->SetMinSize(wxSize(75, -1));

  const std::vector<wxString> lab_list = {L""};
  lab_ctrl_ = new wxChoice(this, kIdLabFilter,
    wxDefaultPosition, wxDefaultSize, lab_list, wxCB_SORT,
    wxGenericValidator(&lab_filter_));
  lab_ctrl_->SetMinSize(wxSize(100, -1));

  auto* name_sizer = new wxBoxSizer(wxVERTICAL);
  auto* name_label = new wxStaticText(this, wxID_ANY, L"Name:");
  name_sizer->Add(name_label, 0, wxALL, 2);
  name_sizer->Add(name_ctrl_, 0, wxALL, 2);

  auto* unit_sizer = new wxBoxSizer(wxVERTICAL);
  auto* unit_label = new wxStaticText(this, wxID_ANY, L"Unit:");
  unit_sizer->Add(unit_label, 0, wxALL, 2);
  unit_sizer->Add(unit_ctrl_, 0, wxALL, 2);

  auto* source_sizer = new wxBoxSizer(wxVERTICAL);
  auto* source_label = new wxStaticText(this, wxID_ANY, L"Source:");
  source_sizer->Add(source_label, 0, wxALL, 2);
  source_sizer->Add(source_ctrl_, 0, wxALL, 2);

  auto* lab_sizer = new wxBoxSizer(wxVERTICAL);
  auto* lab_label = new wxStaticText(this, wxID_ANY, L"Selection List:");
  lab_sizer->Add(lab_label, 0, wxALL, 2);
  lab_sizer->Add(lab_ctrl_, 0, wxALL, 2);

  auto* filter_sizer = new wxBoxSizer(wxHORIZONTAL);
  filter_sizer->Add(name_sizer, 0, wxALL, 2);
  filter_sizer->Add(unit_sizer, 0, wxALL, 2);
  filter_sizer->Add(source_sizer, 0, wxALL, 2);
  filter_sizer->Add(lab_sizer, 1, wxALL | wxGROW, 2);

  auto* main_sizer = new wxBoxSizer(wxVERTICAL);
  main_sizer->Add(list_, 1 , wxALL | wxGROW,0);
  main_sizer->Add(filter_sizer, 0, wxALL, 2);

  SetSizerAndFit(main_sizer);
}

MdfDocument* SignalWindow::GetDoc() const {
  const auto* notebook = dynamic_cast<ConfigWindow*>(GetParent());
  return notebook != nullptr ? notebook->GetDoc() : nullptr;

}

ChildFrame* SignalWindow::GetChildFrame() const {
  const auto* notebook = dynamic_cast<ConfigWindow*>(GetParent());
  return notebook != nullptr ? notebook->GetChildFrame() : nullptr;
}

void SignalWindow::Redraw() const {
  if (list_ != nullptr) {
    list_->Redraw();
  }
}
} // mdf::viewer