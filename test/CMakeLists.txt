# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

project(TestMdf
        VERSION 1.0
        DESCRIPTION "Google unit tests for the mdf library"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include("../script/boost.cmake")
include("../script/utillib.cmake")

add_executable(test_mdf
        test_conversion.cpp
        testcrypto.cpp
        testread.cpp testread.h
        testwrite.cpp testwrite.h
        testmdffile.cpp
        testmetadata.cpp
        testzlib.cpp testzlib.h)

target_include_directories(test_mdf PRIVATE
        ../include
        ../src
        ${utillib_SOURCE_DIR}/include
        ${GTEST_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS})

if (MINGW)
    target_link_options(test_mdf PRIVATE -static -fstack-protector)
elseif (MSVC)
    target_compile_definitions(test_mdf PRIVATE -D_WIN32_WINNT=0x0A00)
endif ()

target_link_libraries(test_mdf PRIVATE mdf)
target_link_libraries(test_mdf PRIVATE util)
target_link_libraries(test_mdf PRIVATE expat)
target_link_libraries(test_mdf PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_mdf PRIVATE ${ZLIB_LIBRARIES})
target_link_libraries(test_mdf PRIVATE ${GTEST_BOTH_LIBRARIES})

if (WIN32)
    #target_link_libraries(test_mdf PRIVATE iconv)
    target_link_libraries(test_mdf PRIVATE ws2_32)
    target_link_libraries(test_mdf PRIVATE mswsock)
endif ()

include(GoogleTest)
gtest_discover_tests(test_mdf)


