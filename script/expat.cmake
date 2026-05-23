# Copyright 2026 Ingemar Hedvall
# SPDX-License-Identifier: MIT
include(CMakePrintHelpers)

if (NOT BUILD_SHARED_LIBS)
    set(EXPAT_USE_STATIC_LIBS ON)
endif()

if (NOT EXPAT_FOUND)
    find_package(EXPAT)
    if (NOT EXPAT_FOUND)
        set(EXPAT_ROOT ${COMP_DIR}/expat/master)
        find_package(EXPAT REQUIRED)
    endif()
endif()

cmake_print_variables(EXPAT_FOUND
                      EXPAT_VERSION
                      EXPAT_INCLUDE_DIRS
                      EXPAT_LIBRARIES
                      EXPAT_USE_STATIC_LIBS)
cmake_print_properties(TARGETS EXPAT::EXPAT
                       PROPERTIES LOCATION
                                  INTERFACE_INCLUDE_DIRECTORIES)

