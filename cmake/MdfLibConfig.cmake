include(CMakeFindDependencyMacro)

find_dependency(ZLIB)
find_dependency(EXPAT)

include("${CMAKE_CURRENT_LIST_DIR}/MdfLibTargets.cmake")
