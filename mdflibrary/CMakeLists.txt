# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT

if (MSVC)
    add_library(mdflibrary SHARED
            src/MdfExport.cpp
            src/AssemblyInfo.cpp
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
  add_library(mdflibrary SHARED src/MdfExport.cpp)
endif()

target_link_libraries(mdflibrary PRIVATE mdf)

target_include_directories(
  mdflibrary PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

set(MDFLIBRARY_PUBLIC_HEADERS 
            ../include/mdflibrary/MdfExport.h
            ../include/mdflibrary/MdfAttachment.h
            ../include/mdflibrary/MdfChannel.h
            ../include/mdflibrary/MdfChannelConversion.h
            ../include/mdflibrary/MdfChannelGroup.h
            ../include/mdflibrary/MdfChannelObserver.h
            ../include/mdflibrary/MdfDataGroup.h
            ../include/mdflibrary/MdfETag.h
            ../include/mdflibrary/MdfEvent.h
            ../include/mdflibrary/MdfFile.h
            ../include/mdflibrary/MdfFileHistory.h
            ../include/mdflibrary/MdfHeader.h
            ../include/mdflibrary/MdfMetaData.h
            ../include/mdflibrary/MdfReader.h
            ../include/mdflibrary/MdfSourceInformation.h
            ../include/mdflibrary/MdfWriter.h)
set_target_properties(mdflibrary PROPERTIES PUBLIC_HEADER
                                            "${MDFLIBRARY_PUBLIC_HEADERS}")
get_target_property(PH1 mdflibrary PUBLIC_HEADER)
message(STATUS "MDFLibrary Target Includes: " "${PH1}")

if(WIN32)
  install(
    TARGETS mdflibrary
    EXPORT MdfLibraryTargets
    LIBRARY DESTINATION mdflibrary/lib
    ARCHIVE DESTINATION mdflibrary/lib
    RUNTIME DESTINATION mdflibrary/bin
    PUBLIC_HEADER DESTINATION mdflibrary/include/mdflibrary)
else()
  install(
    TARGETS mdflibrary
    EXPORT MdfLibraryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mdflibrary)
endif()
