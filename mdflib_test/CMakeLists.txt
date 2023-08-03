# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

enable_testing()

project(
  TestMdf
  VERSION 1.0
  DESCRIPTION "Google unit tests for the mdf library"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_mdf
        src/testcrypto.cpp
        src/testread.cpp src/testread.h
        src/testwrite.cpp src/testwrite.h
        src/testmdffile.cpp
        src/testmetadata.cpp
        src/testzlib.cpp src/testzlib.h
        src/util/ilogger.cpp src/util/ilogger.h
        src/util/logconfig.cpp src/util/logconfig.h
        src/util/logconsole.cpp src/util/logconsole.h
        src/util/logfile.cpp src/util/logfile.h
        src/util/logging.cpp src/util/logging.h
        src/util/logmessage.cpp src/util/logmessage.h
        src/util/logstream.cpp src/util/logstream.h
        src/util/stringutil.cpp src/util/stringutil.h
        src/util/timestamp.cpp src/util/timestamp.h)

target_include_directories(test_mdf PRIVATE ../include ../mdflib/src)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(test_mdf PRIVATE GTest::gtest GTest::gtest_main
                                       GTest::gmock GTest::gmock_main)

target_link_libraries(test_mdf PRIVATE mdf)

add_test(AllTestsInMain main)
