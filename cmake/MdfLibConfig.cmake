include(CMakeFindDependencyMacro)

find_dependency(ZLIB)
find_dependency(expat CONFIG)

include("${CMAKE_CURRENT_LIST_DIR}/MdfLibTargets.cmake")
