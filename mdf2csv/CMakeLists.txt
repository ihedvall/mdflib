# Copyright 2024 Ingemar Hedvall
# SPDX-License-Identifier: MIT

project(MdfToCsv
        VERSION 1.0
        DESCRIPTION "Converts an MDF file to one or more CSV files"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if (MDF_BUILD_TEST)
    add_subdirectory(test)
endif()

add_executable(mdf2csv
    src/main.cpp
        src/programargument.cpp
        src/programargument.h
        src/createfilelist.cpp
        src/createfilelist.h
        src/csvwriter.cpp
        src/csvwriter.h
        src/logtoconsole.cpp
        src/logtoconsole.h)

target_include_directories(mdf2csv PRIVATE ${XTENSOR_INCLUDE_DIRS})

if(MSVC)
    target_compile_definitions(mdf2csv PRIVATE
            _WIN32_WINNT=0x0A00
            _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
            _CRT_SECURE_NO_WARNINGS
            _SILENCE_CXX20_U8PATH_DEPRECATION_WARNING)
elseif(MINGW)
    target_link_options(mdf2csv PRIVATE -mwindows -static -fstack-protector)
endif()

target_link_libraries(mdf2csv PRIVATE mdf)


if(VCPKG)
    find_package(expat CONFIG REQUIRED)
    target_link_libraries(mdf2csv PRIVATE expat::expat)

    find_package(ZLIB REQUIRED)
    target_link_libraries(mdf2csv PRIVATE ZLIB::ZLIB)

    find_package(boost_program_options REQUIRED CONFIG)
    target_link_libraries(mdf2csv PRIVATE Boost::program_options)
else()
    include("../script/zlib.cmake")
    include("../script/expat.cmake")
    include("../script/boost.cmake")

    target_include_directories(mdf2csv PRIVATE ${Boost_INCLUDE_DIRS})

    target_compile_definitions(mdf2csv PRIVATE XML_STATIC)

    target_link_libraries(mdf2csv PRIVATE ${EXPAT_LIBRARIES})
    target_link_libraries(mdf2csv PRIVATE ${Boost_LIBRARIES})
    target_link_libraries(mdf2csv PRIVATE ${ZLIB_LIBRARIES})

endif()
