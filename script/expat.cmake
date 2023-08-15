# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT EXPAT_FOUND)
    set(EXPAT_USE_STATIC_LIBS ON)
    if (COMP_DIR)
        set(EXPAT_ROOT ${COMP_DIR}/expat/master)
    endif()

    find_package(EXPAT REQUIRED)
endif()

message(STATUS "EXPAT Found: "  ${EXPAT_FOUND})
message(STATUS "EXPAT Include Dirs: "  ${EXPAT_INCLUDE_DIRS})
message(STATUS "EXPAT Libraries: " ${EXPAT_LIBRARIES})

