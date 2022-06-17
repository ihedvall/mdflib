# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT

if (NOT wxWidgets_FOUND )
    find_package(wxWidgets COMPONENTS adv core base)
endif()

if (NOT wxWidgets_FOUND)
    if (NOT wxWidgets_ROOT_DIR)
        set(wxWidgets_ROOT_DIR ${COMP_DIR}/wxwidgets/master)
    endif()

    if (MINGW)
        set(wxWidgets_LIB_DIR ${COMP_DIR}/wxwidgets/master/lib/gcc_x64_lib)
    endif()

    find_package(wxWidgets COMPONENTS adv core base)
    include(${wxWidgets_USE_FILE})
    message(STATUS "wxWidgets Found: " ${wxWidgets_FOUND})
    message(STATUS "wxWidgets Include Dirs: " ${wxWidgets_INCLUDE_DIRS})
    message(STATUS "wxWidgets Library Dirs: " ${wxWidgets_LIBRARY_DIRS})
    message(STATUS "wxWidgets Libraries: " ${wxWidgets_LIBRARIES})
    message(STATUS "wxWidgets Include File: " ${wxWidgets_USE_FILE})
    message(STATUS "wxWidgets Definitions: " ${wxWidgets_DEFINITIONS})
    message(STATUS "wxWidgets Debug Definitions: " ${wxWidgets_DEFINITIONS_DEBUG})
    message(STATUS "wxWidgets CXX Flags: " ${wxWidgets_CXX_FLAGS})
endif()





