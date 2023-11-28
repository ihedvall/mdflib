# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT GTest_FOUND)
    set(GTEST_MSVC_SEARCH MT)
    if (COMP_DIR)
        # Ignore the error message that GTest_ROOT should be used. It has to be capital letters
        set(GTEST_ROOT ${COMP_DIR}/googletest/master)
    endif()

    find_package(GTest)
    message(STATUS "GTest Found: " ${GTest_FOUND})
    message(STATUS "GTest Include Dirs: " ${GTEST_INCLUDE_DIRS})
    message(STATUS "GTest Libraries: " ${GTEST_LIBRARIES})
    message(STATUS "GTest Main Libraries: " ${GTEST_MAIN_LIBRARIES})
    message(STATUS "GTest Both Libraries: " ${GTEST_BOTH_LIBRARIES})
endif()
