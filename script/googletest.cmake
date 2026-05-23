# Copyright 2026 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include(FetchContent)
include(CMakePrintHelpers)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG HEAD
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest." FORCE)

FetchContent_MakeAvailable(googletest)
cmake_print_variables(googletest_POPULATED
                      googletest_SOURCE_DIR
                      googletest_BINARY_DIR
                      GTest_FOUND)
cmake_print_properties(TARGETS GTest::gtest_main
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES)