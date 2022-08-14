# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT ZLIB_FOUND)
    SET(ZLIB_USE_STATIC_LIBS ON)
    set(ZLIB_ROOT ${COMP_DIR}/zlib/master)

    find_package(ZLIB)
    message(STATUS "ZLIB Found (Try 1): " ${ZLIB_FOUND})
    message(STATUS "ZLIB Version: " ${ZLIB_VERSION_STRING})
    message(STATUS "ZLIB Include Dirs: " ${ZLIB_INCLUDE_DIRS})
    message(STATUS "ZLIB Libraries: " ${ZLIB_LIBRARIES})

    if (NOT ZLIB_FOUND)
        find_package(ZLIB REQUIRED)
        message(STATUS "ZLIB Found (Try 2): " ${ZLIB_FOUND})
        message(STATUS "ZLIB Version: " ${ZLIB_VERSION_STRING})
        message(STATUS "ZLIB Include Dirs: " ${ZLIB_INCLUDE_DIRS})
        message(STATUS "ZLIB Libraries: " ${ZLIB_LIBRARIES})
    endif()

endif()