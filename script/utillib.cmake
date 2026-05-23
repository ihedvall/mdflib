# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)
include(CMakePrintHelpers)

FetchContent_Declare(utillib
        GIT_REPOSITORY https://github.com/ihedvall/utillib.git
        GIT_TAG HEAD)
set(UTIL_DOC OFF)
set(UTIL_TEST OFF)
set(UTIL_TOOLS OFF)

FetchContent_MakeAvailable(utillib)
cmake_print_variables(utillib_POPULATED utillib_SOURCE_DIR utillib_BINARY_DIR)
cmake_print_properties(TARGETS util PROPERTIES INCLUDE_DIRECTORIES LINK_DIRECTORIES LINK_LIBRARIES)

# hwinfo (fetched by utillib) unconditionally declares all Windows symbols with
# __declspec(dllimport) unless HWINFO_EXPORTS is defined, even for static builds.
# When BUILD_SHARED_LIBS is OFF (static vcpkg triplets), hwinfo builds STATIC and
# never defines HWINFO_EXPORTS, so MSVC sees dllimport declarations with local
# definitions — C4273, promoted to error by hwinfo's own /WX flag.
# Define HWINFO_EXPORTS on each sub-target to restore consistent dllexport behaviour.
if(MSVC)
  foreach(_hwinfo_target
      hwinfo hwinfo_battery hwinfo_cpu hwinfo_disk hwinfo_gpu
      hwinfo_mainboard hwinfo_network hwinfo_os hwinfo_ram)
    if(TARGET ${_hwinfo_target})
      get_target_property(_hwinfo_type ${_hwinfo_target} TYPE)
      if(_hwinfo_type STREQUAL "INTERFACE_LIBRARY")
        target_compile_definitions(${_hwinfo_target} INTERFACE HWINFO_EXPORTS)
      else()
        target_compile_definitions(${_hwinfo_target} PRIVATE HWINFO_EXPORTS)
      endif()
    endif()
  endforeach()
endif()
