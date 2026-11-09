/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <wx/wx.h>
#include <boost/asio.hpp>


namespace mdf::viewer {

class MdfViewer : public wxApp {
 public:

  bool OnInit() override;
  int OnExit() override;

  void OpenFile(const std::string& filename);

  // ANSI coding
  const std::string& GetMyTempDir() const {
    return my_temp_dir_;
  }

  const std::string& GnuPlot() const {
    return gnuplot_;
  }

  Appearance GetAppearance() const { return appearance_; }
  boost::asio::io_context ctx_;
 private:
  std::string notepad_; ///< Path to notepad.exe if it exist
  std::string gnuplot_; ///< Path to gnuplot.exe if it exist
  std::string my_temp_dir_; ///< Temporary directory for this application that is removed at exit
  Appearance appearance_ = Appearance::System;

  void OnOpenLogFile(wxCommandEvent& event);
  void OnUpdateOpenLogFile(wxUpdateUIEvent& event);
  void OnGnuPlotDownloadPage(wxCommandEvent& event);
  void OnUpdateGnuPlotDownloadPage(wxUpdateUIEvent& event);
  wxDECLARE_EVENT_TABLE();
};
} // namespace

wxDECLARE_APP(mdf::viewer::MdfViewer);
