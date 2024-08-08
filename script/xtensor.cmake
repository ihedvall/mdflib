# Copyright 2024 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

FetchContent_Declare(xtl
        GIT_REPOSITORY https://github.com/xtensor-stack/xtl.git
        GIT_TAG HEAD
)

FetchContent_Declare(xtensor
        GIT_REPOSITORY https://github.com/xtensor-stack/xtensor.git
        GIT_TAG HEAD
)

FetchContent_MakeAvailable(xtl xtensor)

set(XTENSOR_INCLUDE_DIRS ${xtl_SOURCE_DIR}/include ${xtensor_SOURCE_DIR}/include)

message(STATUS "XTL Populated: " ${xtl_POPULATED})
message(STATUS "XTL Source: " ${xtl_SOURCE_DIR})
message(STATUS "XTL Binary: " ${xtl_BINARY_DIR})

message(STATUS "XTENSOR Populated: " ${xtensor_POPULATED})
message(STATUS "XTENSOR Source: " ${xtensor_SOURCE_DIR})
message(STATUS "XTENSOR Binary: " ${xtensor_BINARY_DIR})

message(STATUS "XTENSOR Include Dirs: " ${XTENSOR_INCLUDE_DIRS})