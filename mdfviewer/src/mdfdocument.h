/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <wx/wx.h>
#include <wx/docview.h>
#include "mdf/mdfreader.h"
#include "mdf/mdffile.h"
#include "../../mdflib/src/iblock.h"
#include "../../mdflib/src/mdf3file.h"
#include "../../mdflib/src/mdf4file.h"

namespace mdf::viewer {

class MdfDocument : public wxDocument {
 public:
  MdfDocument() = default;
  ~MdfDocument() override = default;

  bool OnOpenDocument(const wxString &filename) override;

  [[nodiscard]] const mdf::MdfFile* GetFile() const {
    return !reader_  ? nullptr : reader_->GetFile();
  }
  [[nodiscard]] const mdf::MdfReader* GetReader() const {
    return reader_.get();
  }

  void SetSelectedBlockId(fpos_t id, fpos_t parent_id, fpos_t grand_parent_id) {
    selected_id_ = id;
    parent_id_ = parent_id;
    grand_parent_id_ = grand_parent_id;
  }

  fpos_t GetSelectedBlockId() const {
    return selected_id_;
  }

  fpos_t GetParentBlockId() const {
    return parent_id_;
  }

  fpos_t GetGrandParentBlockId() const {
    return grand_parent_id_;
  }

  const mdf::detail::IBlock* GetBlock(fpos_t id) const; ///< Returns a block pointer by block index.

 private:
  std::unique_ptr<mdf::MdfReader> reader_;
  fpos_t selected_id_ = 64; ///< The selected items block index (file position).
  fpos_t parent_id_ = -1; ///< The parent index of the selected block.
  fpos_t grand_parent_id_ = -1; ///< The grand parent index of the selected block.

  void OnSaveAttachment(wxCommandEvent& event);
  void OnUpdateSaveAttachment(wxUpdateUIEvent& event);

  void OnShowGroupData(wxCommandEvent& event);
  void OnUpdateShowGroupData(wxUpdateUIEvent& event);

  void OnShowChannelData(wxCommandEvent& event);
  void OnUpdateShowChannelData(wxUpdateUIEvent& event);

  void OnPlotChannelData(wxCommandEvent& event);
  void OnUpdatePlotChannelData(wxUpdateUIEvent& event);
  wxDECLARE_DYNAMIC_CLASS(MdfDocument);
  wxDECLARE_EVENT_TABLE();
};



}



