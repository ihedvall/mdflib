# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT


project(
  TestMdf
  VERSION 1.0
  DESCRIPTION "Google unit tests for the mdf library"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_mdf
        src/main.cpp
        src/testcrypto.cpp
        src/testread.cpp src/testread.h
        src/testwrite.cpp src/testwrite.h
        src/testmdffile.cpp
        src/testmetadata.cpp src/testmetadata.h
        src/testzlib.cpp src/testzlib.h
        src/util/ilogger.cpp src/util/ilogger.h
        src/util/logconfig.cpp src/util/logconfig.h
        src/util/logconsole.cpp src/util/logconsole.h
        src/util/logfile.cpp src/util/logfile.h
        src/util/logging.cpp src/util/logging.h
        src/util/logmessage.cpp src/util/logmessage.h
        src/util/logstream.cpp src/util/logstream.h
        src/util/stringutil.cpp src/util/stringutil.h
        src/util/timestamp.cpp src/util/timestamp.h
        src/testhelper.cpp
        src/testchannel.cpp
        src/testchannelarray.cpp
        src/testchannelarray.h
        src/testqueue.cpp
        src/testiostream.cpp
        src/testmdfblock.cpp
        src/testbuslogger.cpp
        src/testbuslogger.h
        src/testmultipledgwrite.cpp
        src/testmultipledgwrite.h
        src/testtimestamp.cpp
        src/testtimestamp.h
        src/testhdcomment.cpp
        src/testcccomment.cpp
        src/testcncomment.cpp
        src/testevcomment.cpp
        src/testfhcomment.cpp
        src/testsicomment.cpp
        src/testmostlogger.cpp
        src/testmostlogger.h
        src/testflexraylogger.cpp
        src/testflexraylogger.h
        src/testlinlogger.cpp
        src/testlinlogger.h
        src/testethlogger.cpp
        src/testethlogger.h)

target_include_directories(test_mdf PRIVATE ../include ../mdflib/src)

if(VCPKG)
  find_package(boost_endian REQUIRED CONFIG)
  target_link_libraries(test_mdf PRIVATE Boost::endian)

  find_package(GTest CONFIG REQUIRED)
  target_link_libraries(test_mdf PRIVATE GTest::gtest GTest::gtest_main
                                        GTest::gmock GTest::gmock_main)
else()
  include("../script/googletest.cmake")
  include("../script/boost.cmake")
  target_include_directories(test_mdf PRIVATE ${Boost_INCLUDE_DIRS})

  target_link_libraries(test_mdf PRIVATE ${Boost_LIBRARIES})

  target_link_libraries(test_mdf PRIVATE GTest::gtest GTest::gtest_main)
endif()

if(MSVC)
  target_compile_definitions(test_mdf PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(test_mdf PRIVATE mdf)

include(GoogleTest)
gtest_discover_tests(test_mdf)
