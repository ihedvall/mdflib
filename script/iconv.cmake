# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT Iconv_FOUND)
    find_package(Iconv)
endif()

message(STATUS "ICONV Found: "  ${Iconv_FOUND})
message(STATUS "ICONV Include Dirs: "  ${Iconv_INCLUDE_DIRS})
message(STATUS "ICONV Libraries: " ${Iconv_LIBRARIES})
message(STATUS "ICONV Version: " ${Iconv_VERSION})