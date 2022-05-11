# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
set (Boost_ROOT "K:/boost/boost_1_79_0")
if (NOT Boost_FOUND)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_ARCHITECTURE -x64)
    set(Boost_NO_WARN_NEW_VERSIONS ON)
    set(Boost_DEBUG OFF)

    find_package(Boost REQUIRED COMPONENTS filesystem system locale program_options)
endif()
message(STATUS "Boost Found: " ${Boost_FOUND})
message(STATUS "Boost Version: " ${Boost_VERSION_STRING})
message(STATUS "Boost Include Dirs: " ${Boost_INCLUDE_DIRS})
message(STATUS "Boost Library Dirs: " ${Boost_LIBRARY_DIRS})
message(STATUS "Boost Libraries: " ${Boost_LIBRARIES})
