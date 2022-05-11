include (FetchContent)
FetchContent_Declare(utillib
        GIT_REPOSITORY https://github.com/ihedvall/utillib.git
        GIT_TAG HEAD)
FetchContent_MakeAvailable(utillib)
message(STATUS "UTILLIB Populated: " ${utillib_POPULATED})
message(STATUS "UTILLIB Source Dir: " ${utillib_SOURCE_DIR})
message(STATUS "UTILLIB Binary Dir: " ${utillib_BINARY_DIR})