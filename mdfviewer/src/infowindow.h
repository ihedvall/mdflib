/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "wx/listctrl.h"

#include "mdfblock.h"
#include "mdf/iheader.h"

namespace mdf::viewer {

class MdfDocument;
class ConfigWindow;

class InfoWindow : public wxListView {
public:
  InfoWindow(wxWindow* parent, wxWindowID id);
  [[nodiscard]] MdfDocument* GetDoc() const;
  [[nodiscard]] const IHeader* GetHeader() const;
  [[nodiscard]] const detail::MdfBlock* GetCurrentBlock() const;
  [[nodiscard]] const detail::MdfBlock* GetBlock(int64_t block_index) const;
  [[nodiscard]] ConfigWindow* GetConfigWindow() const;
  void Redraw();
private:
  void RedrawConfig();
  void RedrawHistory();
  void RedrawMeasurement();
  void RedrawEvent();
  void RedrawAttachment();
  void RedrawHierarchy();
  void RedrawBlock();
  void DrawPropertyList(const detail::BlockPropertyList& property_list);

  void OnListItemActivated(wxListEvent& event);
};

}  // namespace mdf::viewer

