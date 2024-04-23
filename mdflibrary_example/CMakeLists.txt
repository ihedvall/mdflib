# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
project(
  mdflibraryexample
  VERSION 1.0
  DESCRIPTION "MdfLibraryTest"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(mdflibraryexample src/test.cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(mdflibraryexample PROPERTIES LINK_FLAGS
                                                  "-Wl,-rpath,'$ORIGIN'")
endif()

target_link_libraries(mdflibraryexample PRIVATE mdflibrary pthread)

target_include_directories(
  mdflibrary PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
