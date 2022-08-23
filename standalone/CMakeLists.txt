cmake_minimum_required(VERSION 3.18..3.22)

project(
  BinaryStandalone
  VERSION 1.0
  LANGUAGES CXX
)
# SET(CMAKE_CXX_STANDARD 20)
message(STATUS "Building BinaryStandalone")

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)
include(../cmake/functions.cmake)
# CPMAddPackage("gh:p-ranav/argparse@2.4") the install option has to be explicitly set to allow
# installation
CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage(NAME binary SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../library)
find_package(spdlog REQUIRED)

# ---- Create standalone executable ----

file(GLOB source CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
file(GLOB header CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp)

# Add standalone executable sv2nl
file(GLOB sv2nl_source CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/sv2nl/source/*.cpp)
file(GLOB sv2nl_header CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/sv2nl/source/*.hpp)

add_executable(${PROJECT_NAME} ${source} ${header} ${sv2nl_source} ${sv2nl_header})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "sv2nl")
target_link_libraries(${PROJECT_NAME} PRIVATE binary::binary cxxopts stdc++fs)
target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/sv2nl/include
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
