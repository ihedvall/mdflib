# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
project(mdflibrary
        VERSION 2.0
        DESCRIPTION "Interface against MDF 3/4 files"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILEON ON)

if (WIN32)
    add_library(mdflibrary SHARED
            src/AssemblyInfo.cpp
            src/MdfExport.hpp
            src/MdfAttachment.cpp src/MdfAttachment.h
            src/MdfChannel.cpp src/MdfChannel.h
            src/MdfChannelArray.cpp src/MdfChannelArray.h
            src/MdfChannelConversion.cpp src/MdfChannelConversion.h
            src/MdfChannelGroup.cpp src/MdfChannelGroup.h
            src/MdfChannelObserver.cpp src/MdfChannelObserver.h
            src/MdfDataGroup.cpp src/MdfDataGroup.h
            src/MdfETag.cpp src/MdfETag.h
            src/MdfEvent.cpp src/MdfEvent.h
            src/MdfFile.cpp src/MdfFile.h
            src/MdfFileHistory.cpp src/MdfFileHistory.h
            src/MdfHeader.cpp src/MdfHeader.h
            src/mdflibrary.cpp src/mdflibrary.h
            src/MdfMetaData.cpp src/MdfMetaData.h
            src/MdfReader.cpp src/MdfReader.h
            src/MdfSourceInformation.cpp src/MdfSourceInformation.h
            src/mdfwriter.cpp src/mdfwriter.h
            )
    set_target_properties(mdflibrary PROPERTIES COMMON_LANGUAGE_RUNTIME "")
else()
    add_library(mdflibrary SHARED MdfExport.hpp)
endif()

target_include_directories(mdflibrary PRIVATE mdf)
target_include_directories(mdflibrary PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")

target_link_libraries(mdflibrary PRIVATE mdf)
