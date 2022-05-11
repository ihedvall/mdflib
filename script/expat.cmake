# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
set(EXPAT_ROOT "k:/expat/master" CACHE PATH "The SQLite library is required for the build")
if (NOT EXPAT_FOUND)
    find_package(EXPAT REQUIRED)
endif()

message(STATUS "EXPAT Found: "  ${EXPAT_FOUND})
message(STATUS "EXPAT Include Dirs: "  ${EXPAT_INCLUDE_DIRS})
message(STATUS "EXPAT Libraries: " ${EXPAT_LIBRARIES})
