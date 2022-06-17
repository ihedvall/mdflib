/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "channelobserverlistview.h"

namespace mdf::viewer {

ChannelObserverListView::ChannelObserverListView(wxWindow *parent, wxWindowID win_id, const wxPoint &pos,
                                                 const wxSize &size, long style)
: wxListView(parent,win_id,pos,size,style) {

}

ChannelObserverListView::~ChannelObserverListView() {
  if (observer_list_) {
    observer_list_->clear();
  }
  observer_list_.reset();
}

wxString ChannelObserverListView::OnGetItemText(long item, long column) const {
  if (item < 0 || !observer_list_) {
    return "?";
  }
  size_t sample = static_cast<size_t>(item);
  std::ostringstream s;
  if (column > 0) {
    size_t index = static_cast<size_t>(column - 1);
    if (index < observer_list_->size()) {
      const auto& observer = observer_list_->at(index);

      if (observer) {
        std::string value;
        bool valid = observer->GetEngValue(sample, value);
        s << (valid ? value : "*");
        const auto& channel = observer->Channel();
        const auto* cc = channel.ChannelConversion();

        if (cc != nullptr) {
           switch(cc->Type()) {
            case ConversionType::NoConversion:
            case ConversionType::DateConversion:
            case ConversionType::TimeConversion:
              break;

            default:          // Show channel value
              valid = observer->GetChannelValue(sample, value);
              s << " (" << (valid ? value : "*") << ")";
              break;
          }
        }
      } else {
        s << "?";
      }
    }
  } else if (column == 0) {
    s << sample;
  }

  return wxString::FromUTF8(s.str());
}

} // namespace mdf::viewer