# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT EXPAT_FOUND)
    set(EXPAT_USE_STATIC_LIBS ON)

    find_package(EXPAT)
    message(STATUS "EXPAT Found (Try 1): "  ${EXPAT_FOUND})
    message(STATUS "EXPAT Include Dirs: "  ${EXPAT_INCLUDE_DIRS})
    message(STATUS "EXPAT Libraries: " ${EXPAT_LIBRARIES})
    if (NOT EXPAT_FOUND)
        set(EXPAT_ROOT "k:/expat/master" )
        find_package(EXPAT REQUIRED)
        message(STATUS "EXPAT Found (Try 2): "  ${EXPAT_FOUND})
        message(STATUS "EXPAT Include Dirs: "  ${EXPAT_INCLUDE_DIRS})
        message(STATUS "EXPAT Libraries: " ${EXPAT_LIBRARIES})
    endif()
endif()
