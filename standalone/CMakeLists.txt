cmake_minimum_required(VERSION 3.14..3.22)

project(
  Sv2nlStandalone
  VERSION 1.0
  LANGUAGES CXX
)
# SET(CMAKE_CXX_STANDARD 20)
message(STATUS "Building Sv2nlStandalone")

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

CPMAddPackage(NAME Sv2nl SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../library)
find_package(spdlog REQUIRED)

# ---- Create standalone executable ----

add_executable(${PROJECT_NAME} source/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "sv2nl")
target_link_libraries(${PROJECT_NAME} PRIVATE Sv2nl::Sv2nl cxxopts)

# target_link_libraries_system(${PROJECT_NAME} PRIVATE argparse) # suppress unused parameters
# warning
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
