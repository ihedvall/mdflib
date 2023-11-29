# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
project(
  MdfViewer
  VERSION 1.0
  DESCRIPTION "Simple MDF file browser"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(mdfview
        src/mdfviewer.cpp src/mdfviewer.h
        src/mainframe.cpp src/mainframe.h
        src/mdfdocument.cpp src/mdfdocument.h
        src/mdfview.cpp src/mdfview.h
        src/mdfviewer.rc
        src/childframe.cpp src/childframe.h
        src/windowid.h
        src/channelobserverlistview.cpp src/channelobserverlistview.h
        src/channelobserverframe.cpp src/channelobserverframe.h
        src/util/csvwriter.cpp src/util/csvwriter.h
        src/util/ilogger.cpp src/util/ilogger.h
        src/util/logconfig.cpp src/util/logconfig.h
        src/util/logconsole.cpp src/util/logconsole.h
        src/util/logfile.cpp src/util/logfile.h
        src/util/logging.cpp src/util/logging.h
        src/util/logmessage.cpp src/util/logmessage.h
        src/util/logstream.cpp src/util/logstream.h
        src/util/stringutil.cpp src/util/stringutil.h
        src/util/timestamp.cpp src/util/timestamp.h)

target_include_directories(mdfview PRIVATE mdf
                                           "${CMAKE_CURRENT_SOURCE_DIR}/../mdflib/src")

if(MSVC)
  target_link_options(mdfview PRIVATE /SUBSYSTEM:WINDOWS)
  target_compile_definitions(mdfview PRIVATE _WIN32_WINNT=0x0A00)
elseif(MINGW)
  target_link_options(mdfview PRIVATE -mwindows -static -fstack-protector)
endif()

target_link_libraries(mdfview PRIVATE mdf)

if(VCPKG)
find_package(expat CONFIG REQUIRED)
  target_link_libraries(mdfview PRIVATE expat::expat)

  find_package(ZLIB REQUIRED)
  target_link_libraries(mdfview PRIVATE ZLIB::ZLIB)

  find_package(Boost REQUIRED COMPONENTS filesystem locale)
  target_link_libraries(mdfview PRIVATE Boost::boost Boost::filesystem Boost::locale)

  find_package(wxWidgets CONFIG REQUIRED)
  target_link_libraries(mdfview PRIVATE wx::core wx::base)
else()
include("../script/zlib.cmake")
  include("../script/expat.cmake")
  include("../script/wxwidgets.cmake")
  include("../script/boost.cmake")

  target_include_directories(mdfview PRIVATE ${wxWidgets_INCLUDE_DIRS})
  target_include_directories(mdfview PRIVATE ${Boost_INCLUDE_DIRS})

  target_compile_definitions(mdfview PRIVATE XML_STATIC)

  target_link_libraries(mdfview PRIVATE ${EXPAT_LIBRARIES})
  target_link_libraries(mdfview PRIVATE ${wxWidgets_LIBRARIES})
  target_link_libraries(mdfview PRIVATE ${Boost_LIBRARIES})
  target_link_libraries(mdfview PRIVATE ${ZLIB_LIBRARIES})
  target_link_libraries(mdfview PRIVATE ${OPENSSL_CRYPTO_LIBRARY})
endif()


if(MINGW)
  target_link_libraries(mdfview PRIVATE iconv)
endif()

if(WIN32)
  target_link_libraries(mdfview PRIVATE bcrypt)
  target_link_libraries(mdfview PRIVATE ws2_32)
endif()

if(UNIX)
  target_link_libraries(mdfview PRIVATE rt icuuc icui18n)
endif(UNIX)
