# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
set(SQLite3_ROOT "K:/sqlite3/v337/sqlite3" CACHE PATH "The Expat library is required for the build")
if (NOT SQLite3_FOUND)
    set(SQLite3_FOUND TRUE)
    set(SQLite3_INCLUDE_DIRS ${SQLite3_ROOT}/include )
    if (MSVC)
        set(SQLite3_LIBRARIES optimized ${SQLite3_ROOT}/lib/sqlite3.lib debug ${SQLite3_ROOT}/lib/sqlite3d.lib )
    else ()
        set(SQLite3_LIBRARIES optimized ${SQLite3_ROOT}/lib/libsqlite3.a debug ${SQLite3_ROOT}/lib/libsqlite3d.a )
    endif()
endif()

message(STATUS "SQLite3 Found: "  ${SQLite3_FOUND})
message(STATUS "SQLite3 Version: "  ${SQLite3_VERSION})
message(STATUS "SQLite3 Include Dirs: "  ${SQLite3_INCLUDE_DIRS})
message(STATUS "SQLite3 Libraries: " ${SQLite3_LIBRARIES})