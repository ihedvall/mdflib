# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
project(
  mdflibraryexample
  VERSION 1.0
  DESCRIPTION "MdfLibraryTest"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(mdflibraryexample ../include/mdflibrary/MdfExport.h src/test.cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # set_target_properties(mdflibrary PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS
  # "-m32")
  set_target_properties(mdflibraryexample PROPERTIES LINK_FLAGS
                                                  "-Wl,-rpath,'$ORIGIN'")
endif()

target_link_libraries(mdflibraryexample PRIVATE mdflibrary)
