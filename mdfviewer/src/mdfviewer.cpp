/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <filesystem>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

#include <wx/wx.h>
#include <wx/docview.h>
#include <wx/docmdi.h>
#include <wx/config.h>
#include <wx/utils.h>

#include "util/logconfig.h"
#include "util/logstream.h"

#include "mdfviewer.h"
#include "mainframe.h"
#include "mdfdocument.h"
#include "mdfview.h"
#include "windowid.h"

using namespace util::log;

namespace mdf::viewer {

wxIMPLEMENT_APP(MdfViewer);

wxBEGIN_EVENT_TABLE(MdfViewer, wxApp)
  EVT_UPDATE_UI(kIdOpenLogFile,MdfViewer::OnUpdateOpenLogFile)
  EVT_MENU(kIdOpenLogFile, MdfViewer::OnOpenLogFile)
  EVT_UPDATE_UI(kIdGnuPlotDownloadPage,MdfViewer::OnUpdateGnuPlotDownloadPage)
  EVT_MENU(kIdGnuPlotDownloadPage, MdfViewer::OnGnuPlotDownloadPage)
wxEND_EVENT_TABLE()

bool MdfViewer::OnInit() {
  if (!wxApp::OnInit()) {
    return false;
  }
    // Setup system basic configuration
  SetVendorDisplayName("Measurement Data Reporting Server");
  SetVendorName("ReportServer");
  SetAppName("MdfViewer");
  SetAppDisplayName("MDF File Viewer");

  // Set up the log file.
  // The log file will be in %TEMP%/report_server/mdf_viewer.log
  auto& log_config = LogConfig::Instance();
  log_config.Type(LogType::LogToFile);
  log_config.SubDir("report_server");
  log_config.BaseName("mdf_viewer");
  log_config.CreateDefaultLogger();
  LOG_INFO() << "Log File created. Path: " << log_config.GetLogFile();

  // Find the path to the 'notepad.exe'
  try {
    auto np = boost::process::search_path("notepad");
    notepad_ = np.string();
    LOG_INFO() << "Found NotePad. Path: " << notepad_;
  } catch(const std::exception& ) {
    notepad_.clear();
    LOG_INFO() << "NotePad was not found.";
  }

  // Find the path to the 'gnuplot.exe'
  try {
    auto gp = boost::process::search_path("gnuplot");
    gnuplot_ = gp.string();
    LOG_INFO() << "GnuPlot found. Path: " << gnuplot_;
  } catch(const std::exception& ) {
    gnuplot_.clear();
    LOG_INFO() << "GnuPlot was not found.";
  }

  // Create a temporary directory for this application
  try {
    auto temp_dir = std::filesystem::temp_directory_path();
    auto unique = boost::filesystem::unique_path("MdfViewer%%%%");
    temp_dir.append(unique.string());
    std::filesystem::remove_all(temp_dir);
    std::filesystem::create_directories(temp_dir);
    my_temp_dir_ = temp_dir.string();
    LOG_INFO() << "Created a temporary directory. Path: " << my_temp_dir_;
  } catch (const std::exception& error) {
    LOG_ERROR() << "Error when creating temporary directory. Error:" << error.what();
  }


  auto* app_config = wxConfig::Get();
  wxPoint start_pos;
  app_config->Read("/MainWin/X",&start_pos.x, wxDefaultPosition.x);
  app_config->Read("/MainWin/Y",&start_pos.y, wxDefaultPosition.x);
  wxSize start_size;
  app_config->Read("/MainWin/XWidth",&start_size.x, 1200);
  app_config->Read("/MainWin/YWidth",&start_size.y, 800);
  bool maximized = false;
  app_config->Read("/MainWin/Max",&maximized, maximized);

  auto* doc_manager = new wxDocManager;
  new wxDocTemplate(doc_manager, "MDF File","*.mf4;*.mdf;*.mf3;*.dat","",
                                         "mf4;mdf;mf3;dat","MDFViewer","MDF Viewer",
                                         wxCLASSINFO(MdfDocument), wxCLASSINFO(MdfView));
  auto* frame = new MainFrame(GetAppDisplayName(), start_pos, start_size, maximized);

  frame->Show(true);

  return true;
}

int MdfViewer::OnExit() {
  LOG_INFO() << "Closing application";
  auto* app_config = wxConfig::Get();
  auto* doc_manager = wxDocManager::GetDocumentManager();
  doc_manager->FileHistorySave(*app_config);
  delete doc_manager;
  LOG_INFO() << "Saved file history.";

  try {
    std::filesystem::remove_all(my_temp_dir_);
    LOG_INFO() << "Removed temporary directory. Path: " << my_temp_dir_;
  } catch (const std::exception& error) {
    LOG_ERROR() << "Failed to remove temporary directory. Path: " << my_temp_dir_;
  }

  auto& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
  return wxApp::OnExit();
}

void MdfViewer::OnOpenLogFile(wxCommandEvent& event) {
  auto& log_config = LogConfig::Instance();
  std::string logfile = log_config.GetLogFile();
  OpenFile(logfile);

}

void MdfViewer::OnUpdateOpenLogFile(wxUpdateUIEvent &event) {
  if (notepad_.empty()) {
    event.Enable(false);
    return;
  }

  auto& log_config = LogConfig::Instance();
  std::string logfile = log_config.GetLogFile();
  try {
    std::filesystem::path p(logfile);
    const bool exist = std::filesystem::exists(p);
    event.Enable(exist);
  } catch (const std::exception&) {
    event.Enable(false);
  }
}

void MdfViewer::OnGnuPlotDownloadPage(wxCommandEvent& event) {
  wxLaunchDefaultBrowser("http://www.gnuplot.info/",
                         wxBROWSER_NEW_WINDOW);
}

void MdfViewer::OnUpdateGnuPlotDownloadPage(wxUpdateUIEvent &event) {
   event.Enable( true /* gnuplot_.empty() */ );
}

void MdfViewer::OpenFile(const std::string& filename) const {
  if (!notepad_.empty()) {
    boost::process::spawn(notepad_, filename);
  }
}

}


