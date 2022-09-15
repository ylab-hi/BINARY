# this script adds all subprojects to a single build to allow IDEs understand the full project
# structure.

cmake_minimum_required(VERSION 3.14...3.22)

project(BINARY_All LANGUAGES CXX)
message(STATUS "Building all subprojects for BINARY")
include(cmake/tools.cmake)

option(BINARY_BUILD_STANDALONE "Build standalone" ON)
option(BINARY_BUILD_TESTS "Build tests" ON)
option(BINARY_BUILD_DOCUMENTS "Build DOCUMENTS" OFF)

if(BINARY_BUILD_STANDALONE)
  add_subdirectory(standalone ${CMAKE_BINARY_DIR}/standalone)
else()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/library)
endif()

if(BINARY_BUILD_TESTS)
  # needed to generate test target
  enable_testing()
  add_subdirectory(test ${CMAKE_BINARY_DIR}/test)
endif()

if(BINARY_BUILD_DOCUMENTS)
  add_subdirectory(documentation ${CMAKE_BINARY_DIR}/documentation)
endif()
