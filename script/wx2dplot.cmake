# Copyright 2026 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include(FetchContent)
include(CMakePrintHelpers)

FetchContent_Declare(wx2dplot
        GIT_REPOSITORY https://github.com/ihedvall/wx2dplot.git
        GIT_TAG HEAD
)
FetchContent_MakeAvailable(wx2dplot)

cmake_print_variables(wx2dplot_POPULATED
        wx2dplot_SOURCE_DIR
        wx2dplot_BINARY_DIR)

cmake_print_properties(TARGETS wx2dplot
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_LINK_LIBRARIES)
