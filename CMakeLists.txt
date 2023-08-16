# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)

if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
  set(VCPKG OFF)
else()
  set(VCPKG ON)
endif()

option(MDF_BUILD_SHARED_LIB "Build shared library." OFF)
option(MDF_BUILD_SHARED_LIB_EXAMPLE "Build MdfLibrary Example." OFF)
option(MDF_BUILD_DOC "Build documentation. Requires Doxygen and Release mode." OFF)
option(MDF_BUILD_TOOL "Build tools like the MDF Viewer. Requires WxWidgets." OFF)
if(MDF_BUILD_TOOL AND VCPKG)
  # Boost & wxWidgets is not available for static linking
  string(REPLACE "-static" "" VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}")
  list(APPEND VCPKG_MANIFEST_FEATURES "mdfviewer")
endif()
option(MDF_BUILD_TEST "Build Google unit tests. Requires Google Test." OFF)
if(MDF_BUILD_TEST AND VCPKG)
  # GTest is not available for static linking
  string(REPLACE "-static" "" VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}")
  list(APPEND VCPKG_MANIFEST_FEATURES "mdflibtest")
endif()

project(
  mdflib
  VERSION 2.0
  DESCRIPTION "Interface against MDF 3/4 files"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX d)

add_subdirectory(mdflib)

if(MDF_BUILD_SHARED_LIB)
  add_subdirectory(mdflibrary)
endif()

if(MDF_BUILD_TEST)
  add_subdirectory(mdflib_test)
endif()

if(MDF_BUILD_SHARED_LIB AND MDF_BUILD_SHARED_LIB_EXAMPLE)
  add_subdirectory(mdflibrary_example)
endif()

if(MDF_BUILD_TOOL)
  add_subdirectory(mdfviewer)
endif()

if((CMAKE_BUILD_TYPE MATCHES "^[Rr]elease") AND MDF_BUILD_DOC)
  include("script/doxygen.cmake")
  if(DOXYGEN_FOUND)
    set(DOXYGEN_RECURSIVE NO)
    set(DOXYGEN_REPEAT_BRIEF NO)
    set(DOXYGEN_PROJECT_NAME "MDFLib")
    set(DOXYGEN_HTML_EXTRA_STYLESHEET doxygen/utillib.css)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/manual)
    doxygen_add_docs(
      doc_libmdf ALL include/mdf doxygen
      COMMENT "MDF Library generating API documentation with Doxygen")
  endif()
endif()

# ##############################################################################
# Packaging
# ##############################################################################
# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-a-package-configuration-file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mdflib/mdflibConfigVersion.cmake"
  VERSION ${Upstream_VERSION}
  COMPATIBILITY AnyNewerVersion)

export(
  EXPORT MdfLibTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/mdflib/mdflibTargets.cmake"
  NAMESPACE Upstream::)
configure_file(cmake/MdfLibConfig.cmake
               "${CMAKE_CURRENT_BINARY_DIR}/mdflib/MdfLibConfig.cmake" COPYONLY)

set(ConfigPackageLocation lib/cmake/mdflib)
install(
  EXPORT MdfLibTargets
  FILE MdfLibTargets.cmake
  NAMESPACE Upstream::
  DESTINATION ${ConfigPackageLocation})
install(
  FILES cmake/MdfLibConfig.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/mdflib/mdflibConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
  COMPONENT Devel)
