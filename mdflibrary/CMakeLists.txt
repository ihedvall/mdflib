# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
project(mdflibrary
        VERSION 2.0
        DESCRIPTION "Interface against MDF 3/4 files"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

if (_MSC_VER)
    add_library(mdflibrary SHARED
            src/AssemblyInfo.cpp
            include/mdflibrary/MdfExport.h src/MdfExport.cpp
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
            src/Mdfwriter.cpp src/Mdfwriter.h
            )
    set_target_properties(mdflibrary PROPERTIES COMMON_LANGUAGE_RUNTIME "")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    add_library(mdflibrary SHARED src/MdfExport.cpp)
    set_target_properties(mdflibrary PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

target_include_directories(mdflibrary PRIVATE mdf)

target_link_libraries(mdflibrary PRIVATE mdf)

set(MDF_PUBLIC_HEADERS
include/mdflibrary/MdfExport.h
)
set_target_properties(mdflibrary PROPERTIES PUBLIC_HEADER "${MDF_PUBLIC_HEADERS}")
get_target_property(PH1 mdflibrary PUBLIC_HEADER)
message(STATUS "MDFLibrary Target Includes: " "${PH1}")

if (WIN32)
    install(TARGETS mdflibrary
            EXPORT MdfLibraryTargets
            LIBRARY DESTINATION mdflibrary/lib
            ARCHIVE DESTINATION mdflibrary/lib
            RUNTIME DESTINATION mdflibrary/bin
            PUBLIC_HEADER DESTINATION mdf/include/mdflibrary)
else()
    install(TARGETS mdflibrary
            EXPORT MdfLibraryTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/mdflibrary)
endif()
