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
CPMAddPackage("gh:p-ranav/argparse@2.4")
CPMAddPackage(NAME Sv2nl SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
FIND_PACKAGE(spdlog REQUIRED)

# ---- Create standalone executable ----

add_executable(${PROJECT_NAME} source/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "sv2nl")
target_link_libraries(${PROJECT_NAME} PRIVATE Sv2nl::Sv2nl spdlog::spdlog)

target_link_libraries_system(${PROJECT_NAME} PRIVATE argparse) # suppress unused parameters warning
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)


set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF)
