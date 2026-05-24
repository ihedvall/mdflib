project(
  mdflib_example
  VERSION 1.0
  DESCRIPTION "MDF Lib - DataWriter example"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(datawriter_example src/datawriter_example.cpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(datawriter_example PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()

target_link_libraries(datawriter_example PRIVATE mdf)

target_include_directories(
  datawriter_example PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
