# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include(CMakePrintHelpers)

if (NOT Boost_FOUND)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_ARCHITECTURE -x64)
    set(Boost_NO_WARN_NEW_VERSIONS ON)
    set(Boost_DEBUG OFF)

    find_package(Boost CONFIG COMPONENTS filesystem locale program_options iostreams)
    if (NOT Boost_FOUND)
        if (COMP_DIR)
            set(Boost_ROOT ${COMP_DIR}/boost/latest/lib/cmake)
        endif()
        find_package(Boost REQUIRED CONFIG COMPONENTS filesystem locale program_options iostreams)
    endif()


endif()

cmake_print_variables(
        Boost_FOUND
        Boost_VERSION_STRING
        Boost_INCLUDE_DIRS
        Boost_LIBRARY_DIRS
        Boost_LIBRARIES)

