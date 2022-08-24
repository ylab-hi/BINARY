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

function(add_standalone targetName source)
  message(STATUS "Adding standalone ${targetName}")
  file(GLOB ${targetName}_source CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_LIST_DIR}/${targetName}/source/*.cpp
  )
  add_executable(${targetName} ${source} ${${targetName}_source})
  target_compile_features(${targetName} PRIVATE cxx_std_20)
  target_link_libraries(${targetName} PRIVATE binary::binary cxxopts stdc++fs)
  target_include_directories(
    ${targetName} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
                          ${CMAKE_CURRENT_LIST_DIR}/${targetName}/include
  )
  target_compile_options(${targetName} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)
  set_target_properties(${targetName} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()
# ---- Create standalone executable ----

file(GLOB source CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)

add_standalone(sv2nl ${source})
