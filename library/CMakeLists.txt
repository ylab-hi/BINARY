cmake_minimum_required(VERSION 3.18..3.22)

project(
  binary
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

message(STATUS "Started CMake for  ${PROJECT_NAME} v${PROJECT_VERSION}...")
message(STATUS "Compiler name: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Cmake Binary path: ${CMAKE_BINARY_DIR}")
message(STATUS "Build Dependencies: ${BINARY_BUILD_DEPENDENCIES}")

# ---- Set up Options ----
option(HTSlib_USE_STATIC_LIBS "Use static libs for htslib" OFF)

# ---- Set up dependency----
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include(CPM)

# set(HTSlib_USE_STATIC_LIBS ON)

include(htslib)
find_package(Threads REQUIRED)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.10.0
  OPTIONS "SPDLOG_INSTALL YES"
)

# ---- Add source files ----

add_library(
  ${PROJECT_NAME}
  # headers
  include/binary/exception.hpp
  include/binary/utils.hpp
  include/binary/algorithm/interval_tree.hpp
  include/binary/concepts.hpp
  include/binary/parser/vcf.hpp
  include/binary/algorithm/all.hpp
  include/binary/parser/all.hpp
  # sources
  source/utils.cpp
  include/binary/algorithm/experimental.hpp
  include/binary/algorithm/rb_tree.hpp
)

if(NOT HTSlib_FOUND)
  add_dependencies(${PROJECT_NAME} htslib)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wnon-virtual-dtor -pedantic)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Threads::Threads ${HTSlib_LIBRARIES}
  PUBLIC spdlog::spdlog
)

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${HTSlib_INCLUDE_DIRS}>)

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR include
  INCLUDE_DESTINATION library/include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "spdlog 1.10.0;htslib 1.10.0"
)
