/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "channelobserverframe.h"


namespace mdf::viewer {

ChannelObserverFrame::ChannelObserverFrame(std::unique_ptr<ChannelObserverList>& observer_list, wxWindow *parent,
                                           wxWindowID id, const wxString &title)
: wxFrame(parent, id,title,wxDefaultPosition,parent->GetClientSize(),
          wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT) {
  SetIcon(wxIcon("SUB_ICON", wxBITMAP_TYPE_ICO_RESOURCE));
  list_view_ = new ChannelObserverListView(this);
  list_view_->AppendColumn(wxString::FromUTF8("Sample"));
  size_t samples = 0;
  for ( const auto& channel : *observer_list) {
    std::ostringstream label;
    if (!channel) {
      label << "Alien";
    } else{
      label << channel->Name();
      std::string unit = channel->Unit();
      if (!unit.empty()) {
        label << " [" << unit << "]";
      }
      if (samples < channel->NofSamples()) {
        samples = channel->NofSamples();
      }
    }
    list_view_->AppendColumn(wxString::FromUTF8(label.str()));
  }

  list_view_->ObserverList(observer_list);
  list_view_->SetItemCount(samples);
  for (int col = 0; col < list_view_->GetColumnCount(); ++col) {
    list_view_->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);
  }

}

}