# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
include(FetchContent)
include(CMakePrintHelpers)

if (NOT EXPAT_FOUND)
    FetchContent_Declare(expat
        GIT_REPOSITORY https://github.com/libexpat/libexpat.git
        GIT_TAG  origin/master
        SOURCE_SUBDIR expat
    )
    set(EXPAT_SHARED_LIBS OFF)
    set(EXPAT_NS OFF)
    set(EXPAT_DEBUG_POSTFIX d)
    set(EXPAT_RELEASE_POSTFIX "")
    set(EXPAT_USE_STATIC_LIBS ON)
    FetchContent_MakeAvailable(expat)
    message(STATUS "EXPAT Populated: " ${expat_POPULATED})
    message(STATUS "EXPAT Source Dir: " ${expat_SOURCE_DIR})
    message(STATUS "EXPAT Binary Dir: " ${expat_BINARY_DIR})
    cmake_print_properties(TARGETS expat PROPERTIES INCLUDE_DIRECTORIES  )

endif()