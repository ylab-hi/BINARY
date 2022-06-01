cmake_minimum_required(VERSION 3.15)

project(
    Sv2nlStandalone
    VERSION 1.0
    LANGUAGES CXX
)

# --- Import tools ----

include(../cmake/tools.cmake)


# ---- Dependencies ----

include(../cmake/CPM.cmake)
include(../cmake/functions.cmake)
cpmaddpackage("gh:p-ranav/argparse@2.4")
cpmaddpackage(
    NAME thread-pool
    GITHUB_REPOSITORY DeveloperPaul123/thread-pool
    GIT_TAG 0.5.0 #0cea9c12fb30cb677696c0dce6228594ce26171a change this to latest commit or release tag
    OPTIONS "TP_BUILD_EXAMPLES OFF" "TP_BUILD_BENCHMARKS OFF" "TP_BUILD_TESTS OFF"
)
CPMAddPackage(NAME Sv2nl SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
FIND_PACKAGE(spdlog REQUIRED)

# ---- Create standalone executable ----

add_executable(${PROJECT_NAME} source/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "sv2nl")
target_link_libraries(${PROJECT_NAME} PRIVATE Sv2nl::Sv2nl dp::thread-pool spdlog)

target_link_libraries_system(${PROJECT_NAME} PRIVATE argparse) # suppress unused parameters warning
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)


set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF)
