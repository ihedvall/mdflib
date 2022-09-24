cmake_minimum_required(VERSION 3.20)
project(mdfTestReadExample
        VERSION 1.0
        DESCRIPTION "Test find_package mdf"
        LANGUAGES CXX)
        
find_package(mdf REQUIRED)

add_executable(TestMDFCMake main.cpp)
