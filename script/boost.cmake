# Copyright 2026 Ingemar Hedvall
# SPDX-License-Identifier: MIT
include(CMakePrintHelpers)

if (NOT Boost_FOUND)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_ARCHITECTURE -x64)
    set(Boost_NO_WARN_NEW_VERSIONS ON)
    set(Boost_DEBUG OFF)

    if (COMP_DIR)
        #set(Boost_DIR ${COMP_DIR}/boost/latest/lib/cmake/Boost-1.86.0)
        set(Boost_ROOT ${COMP_DIR}/boost/latest)
    endif()

    find_package(Boost REQUIRED CONFIG COMPONENTS filesystem locale program_options iostreams process)


endif()

cmake_print_variables(Boost_FOUND
                      Boost_VERSION
                      Boost_INCLUDE_DIRS
                      Boost_LIBRARY_DIRS
                      Boost_LIBRARIES )
