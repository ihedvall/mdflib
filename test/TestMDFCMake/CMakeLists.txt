cmake_minimum_required(VERSION 3.20)
project(mdfTestReadExample
        VERSION 1.0
        DESCRIPTION "Test find_package mdf"
        LANGUAGES CXX)
        
find_package(MdfLib REQUIRED)

add_executable(TestMDFCMake main.cpp)
target_link_libraries(TestMDFCMake MdfLibTargets)
target_include_directories(TestMDFCMake PRIVATE ${MdfLibTargets_INCLUDE_DIR})
