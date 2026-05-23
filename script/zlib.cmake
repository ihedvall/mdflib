# Copyright 2026 Ingemar Hedvall
# SPDX-License-Identifier: MIT
include(CMakePrintHelpers)

if (NOT BUILD_SHARED_LIBS)
    set(ZLIB_USE_STATIC_LIBS ON)
endif()

if (NOT ZLIB_FOUND)
    find_package(ZLIB)
    if (NOT ZLIB_FOUND)
        set(ZLIB_ROOT ${COMP_DIR}/zlib/master)
        find_package(ZLIB REQUIRED)
    endif()
endif()

cmake_print_variables(ZLIB_FOUND
                      ZLIB_VERSION
                      ZLIB_INCLUDE_DIRS
                      ZLIB_LIBRARIES
                      ZLIB_USE_STATIC_LIBS)
cmake_print_properties(TARGETS ZLIB::ZLIB
        PROPERTIES LOCATION
        INTERFACE_INCLUDE_DIRECTORIES)