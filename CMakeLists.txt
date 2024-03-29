# this script adds all subprojects to a single build to allow IDEs understand the full project
# structure.

cmake_minimum_required(VERSION 3.14...3.22)

project(BINARY_All LANGUAGES CXX)
message(STATUS "Building all subprojects for BINARY")
include(cmake/tools.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BINARY_BUILD_STANDALONE "Build standalone" ON)
option(BINARY_BUILD_TESTS "Build tests" ON)
option(BINARY_BUILD_DOCUMENTS "Build DOCUMENTS" OFF)
option(BINARY_BUILD_DEPENDENCIES "Build dependencies" ON)

if(BINARY_BUILD_STANDALONE)
  add_subdirectory(standalone ${CMAKE_BINARY_DIR}/standalone)
else()
  add_subdirectory(library ${CMAKE_BINARY_DIR}/library)
endif()

if(BINARY_BUILD_TESTS)
  # needed to generate test target
  enable_testing()
  add_subdirectory(test ${CMAKE_BINARY_DIR}/test)
endif()

if(BINARY_BUILD_DOCUMENTS)
  add_subdirectory(documentation ${CMAKE_BINARY_DIR}/documentation)
endif()
