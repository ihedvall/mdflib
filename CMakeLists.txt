# Copyright 2021 Ingemar Hedvall
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
project(mdflib
        VERSION 1.0
        DESCRIPTION "Interface against MDF 3/4 files"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX d)
include("script/boost.cmake")
include("script/utillib.cmake")
include("script/zlib.cmake")
include("script/expat.cmake")
include("script/openssl.cmake")
include("script/googletest.cmake")
include("script/doxygen.cmake")

add_library(mdf STATIC
        src/iblock.cpp src/iblock.h
        src/mdffile.cpp include/mdf/mdffile.h
        src/idblock.cpp src/idblock.h
        src/hd3block.cpp src/hd3block.h
        src/tx3block.cpp src/tx3block.h
        src/pr3block.cpp src/pr3block.h
        src/tr3block.cpp src/tr3block.h
        src/sr3block.cpp src/sr3block.h
        src/dg3block.cpp src/dg3block.h
        src/cg3block.cpp src/cg3block.h
        src/cn3block.cpp src/cn3block.h
        src/cc3block.cpp src/cc3block.h
        src/cd3block.cpp src/cd3block.h
        src/ce3block.cpp src/ce3block.h
        include/mdf/mdfreader.h src/mdfreader.cpp
        src/mdf3file.cpp src/mdf3file.h
        src/hd4block.cpp src/hd4block.h
        src/tx4block.cpp src/tx4block.h
        src/md4block.cpp src/md4block.h
        src/fh4block.cpp src/fh4block.h
        src/ch4block.cpp src/ch4block.h
        src/at4block.cpp src/at4block.h
        src/ev4block.cpp src/ev4block.h
        src/dg4block.cpp src/dg4block.h
        src/cg4block.cpp src/cg4block.h
        src/si4block.cpp src/si4block.h
        src/cn4block.cpp src/cn4block.h
        src/cc4block.cpp src/cc4block.h
        src/ca4block.cpp src/ca4block.h
        src/dt4block.cpp src/dt4block.h
        src/sr4block.cpp src/sr4block.h
        src/rd4block.cpp src/rd4block.h
        src/sd4block.cpp src/sd4block.h
        src/dl4block.cpp src/dl4block.h
        src/dz4block.cpp src/dz4block.h
        src/hl4block.cpp src/hl4block.h
        src/mdf4file.cpp src/mdf4file.h
        src/datablock.cpp src/datablock.h
        src/datalistblock.cpp src/datalistblock.h
        src/blockproperty.cpp src/blockproperty.h
        src/mdf4timestamp.cpp src/mdf4timestamp.h
        include/mdf/iattachment.h src/iattachment.cpp
        include/mdf/ichannel.h src/ichannel.cpp
        include/mdf/idatagroup.h src/idatagroup.cpp
        include/mdf/ichannelgroup.h src/ichannelgroup.cpp
        include/mdf/isampleobserver.h
        src/channelobserver.h src/channelobserver.cpp
        include/mdf/ichannelobserver.h src/ichannelobserver.cpp
        include/mdf/ichannelconversion.h src/ichannelconversion.cpp
        src/mdf3writer.h src/mdf3writer.cpp
        src/dt3block.cpp src/dt3block.h
        include/mdf/iheader.h
        include/mdf/samplerecord.h src/iheader.cpp src/mdf4writer.cpp src/mdf4writer.h src/mdfwriter.cpp include/mdf/mdfwriter.h src/mdffactory.cpp include/mdf/mdffactory.h src/ifilehistory.cpp include/mdf/ifilehistory.h src/imetadata.cpp include/mdf/imetadata.h src/etag.cpp include/mdf/etag.h)

target_include_directories(mdf PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )
target_include_directories(mdf PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(mdf PRIVATE ${utillib_SOURCE_DIR}/include)
target_compile_definitions(util PRIVATE XML_STATIC)
set(MDF_PUBLIC_HEADERS
        include/mdf/iattachment.h
        include/mdf/ichannel.h
        include/mdf/ichannelconversion.h
        include/mdf/ichannelgroup.h
        include/mdf/ichannelobserver.h
        include/mdf/idatagroup.h
        include/mdf/isampleobserver.h
        include/mdf/mdffile.h
        include/mdf/mdfreader.h
)

set_target_properties(mdf PROPERTIES PUBLIC_HEADER "${MDF_PUBLIC_HEADERS}")
get_target_property(PH1 mdf PUBLIC_HEADER)
message( STATUS "MDF Target Includes: " "${PH1}")

install(TARGETS mdf
        EXPORT MdfLibTargets
        LIBRARY DESTINATION mdf/lib
        ARCHIVE DESTINATION mdf/lib
        RUNTIME DESTINATION mdf/bin
        PUBLIC_HEADER DESTINATION mdf/include/mdf)

if (GTEST_FOUND)
    enable_testing()
    add_subdirectory(test)
endif()

if (DOXYGEN_FOUND AND (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease") )
    set(DOXYGEN_RECURSIVE YES)
    doxygen_add_docs( doc_libmdf ALL
            include/mdf
            COMMENT "MDF Library generating API documentation with Doxygen")
endif()