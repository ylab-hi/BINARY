# this script adds all subprojects to a single build to allow IDEs understand the full project
# structure.

cmake_minimum_required(VERSION 3.14...3.22)

project(BuildAll LANGUAGES CXX)
message(STATUS "Building all subprojects for BINARY")
include(cmake/tools.cmake)

# needed to generate test target
enable_testing()

add_subdirectory(standalone ${CMAKE_BINARY_DIR}/standalone)
add_subdirectory(test ${CMAKE_BINARY_DIR}/test)
#add_subdirectory(documentation ${CMAKE_BINARY_DIR}/documentation)
