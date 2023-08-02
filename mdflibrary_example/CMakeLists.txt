# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
project(mdflibrarytest
        VERSION 1.0
        DESCRIPTION "MdfLibraryTest"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(mdflibrarytest src/test.cpp)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # set_target_properties(mdflibrary PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
        set_target_properties(mdflibrarytest PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()

target_link_libraries(mdflibrarytest PRIVATE mdflibrary)