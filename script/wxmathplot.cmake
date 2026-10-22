# Copyright 2026 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include(FetchContent)
include(CMakePrintHelpers)

FetchContent_Declare(wxmathplot
        GIT_REPOSITORY https://github.com/ihedvall/wxMathPlot.git
        GIT_TAG HEAD
)
FetchContent_MakeAvailable(wxmathplot)

cmake_print_variables(wxmathplot_POPULATED
        wxmathplot_SOURCE_DIR
        wxmathplot_BINARY_DIR)

cmake_print_properties(TARGETS wxmathplot
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                   INTERFACE_LINK_LIBRARIES)
