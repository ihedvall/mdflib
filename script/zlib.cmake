# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
set(ZLIB_ROOT "k:/zlib/master" CACHE PATH "ZLIB is required to build the libmdf and the libutil packages")
if (NOT ZLIB_FOUND)
    find_package(ZLIB REQUIRED )
endif()
message(STATUS "ZLIB Found: " ${ZLIB_FOUND})
message(STATUS "ZLIB Version: " ${ZLIB_VERSION_STRING})
message(STATUS "ZLIB Include Dirs: " ${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLIB Libraries: " ${ZLIB_LIBRARIES})