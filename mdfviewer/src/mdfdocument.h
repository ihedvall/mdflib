/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <memory>

#include <wx/docview.h>
#include <wx/timer.h>
#include <plot/configuration.h>

#include "mdf/readerthread.h"
#include "mdfblock.h"
#include "configdata.h"

namespace mdf::viewer {

class MdfDocument : public wxDocument {
 public:
  MdfDocument();
  ~MdfDocument() override;

  bool OnOpenDocument(const wxString &filename) override;

  const plot::Configuration& GetConfiguration() const {
    return configuration_;
  }

  plot::Configuration& GetConfiguration() {
    return configuration_;
  }

  [[nodiscard]] ReaderList& GetReaderList() {
    return reader_thread_.GetReaderList();
  }
  [[nodiscard]] const ReaderList& GetReaderList() const {
    return reader_thread_.GetReaderList();
  }

  [[nodiscard]] SignalList& GetSignalList() {
    return reader_thread_.GetSignalList();
  }
  [[nodiscard]] const SignalList& GetSignalList() const {
    return reader_thread_.GetSignalList();
  }

  [[nodiscard]] static detail::MdfBlock* GetBlock(const MdfReader* reader,
        int64_t id); ///< Returns a block pointer by block index.

  void SetLastConfigData(const ConfigData& config_data) {
    last_config_data_ = config_data;
  }
  [[nodiscard]] const ConfigData& GetLastConfigData() const {
    return last_config_data_;
  };
 protected:
  bool DoSaveDocument(const wxString& file) override;
  bool DoOpenDocument(const wxString& file) override;
  void SetConfigFilename(const std::wstring& filename);
  void AddMdfFile(const std::wstring& filename, bool add_reader_only);

 private:
  plot::Configuration original_configuration_;;
  plot::Configuration configuration_;
  ReaderThread reader_thread_;
  wxTimer modify_timer_;

  int64_t selected_id_ = 64; ///< The selected items block index (file position).
  int64_t parent_id_ = -1; ///< The parent index of the selected block.
  int64_t grand_parent_id_ = -1; ///< The grand parent index of the selected block.

  ConfigData last_config_data_;
  void OnSaveAttachment(wxCommandEvent& event);
  void OnUpdateSaveAttachment(wxUpdateUIEvent& event);

  void OnShowGroupData(wxCommandEvent& event);
  void OnUpdateShowGroupData(wxUpdateUIEvent& event);

  void OnShowChannelData(wxCommandEvent& event);
  void OnUpdateShowChannelData(wxUpdateUIEvent& event);

  void OnShowSrData(wxCommandEvent& event);
  void OnUpdateShowSrData(wxUpdateUIEvent& event);

  void OnPlotChannelData(wxCommandEvent& event);
  void OnUpdatePlotChannelData(wxUpdateUIEvent& event);

  void OnModifyTimer(wxTimerEvent& event);
  wxDECLARE_DYNAMIC_CLASS(MdfDocument);
  wxDECLARE_EVENT_TABLE();
};



}



