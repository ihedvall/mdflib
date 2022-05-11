# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
set(GTEST_ROOT "k:/googletest/master" CACHE PATH "Google Test is used for all unit tests")
if (NOT GTest_FOUND)
    set(GTEST_MSVC_SEARCH MT)
    find_package(GTest)
    message(STATUS "GTest Found: " ${GTEST})
    message(STATUS "GTest Include Dirs: " ${GTEST_INCLUDE_DIRS})
    message(STATUS "GTest Libraries: " ${GTEST_LIBRARIES})
    message(STATUS "GTest Main Libraries: " ${GTEST_MAIN_LIBRARIES})
    message(STATUS "GTest Both Libraries: " ${GTEST_BOTH_LIBRARIES})
endif()

#        "Include dirs: " ${GTEST_INCLUDE_DIRS} ". "
#        "Libraries: " ${GTEST_LIBRARIES})
#include(FetchContent)
#FetchContent_Declare(googletest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        GIT_TAG master)

# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
#set(BUILD_GTEST ON CACHE BOOL "" FORCE)
#set(INSTALL_GTEST OFF)
#FetchContent_MakeAvailable(googletest)
#message(STATUS "GoogleTest Populated: " ${googletest_POPULATED})
#message(STATUS "GoogleTest Source Dir: " ${googletest_SOURCE_DIR})