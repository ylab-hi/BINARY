cmake_minimum_required(VERSION 3.18...3.22)

project(BinaryTests LANGUAGES CXX)

message(STATUS "Building BinaryTests")
# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

message(STATUS "ENABLE_TEST_COVERAGE: ${ENABLE_TEST_COVERAGE}")
message(STATUS "TEST_INSTALLED_VERSION: ${TEST_INSTALLED_VERSION}")
# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:onqtam/doctest@2.4.9")

CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

if(TEST_INSTALLED_VERSION)
  find_package(binary REQUIRED)
else()
  CPMAddPackage(NAME binary SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../library)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
file(GLOB binary_algorithm CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/source/test_algorithm/*.cpp)
file(GLOB binary_parsers CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/source/test_parser/*.cpp)
file(GLOB binary_utils CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/source/test_utils/*.cpp)

add_executable(${PROJECT_NAME} ${sources} ${binary_algorithm} ${binary_parsers} ${binary_utils})
target_link_libraries(${PROJECT_NAME} doctest::doctest binary::binary)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(
      binary PUBLIC -Wall -Wpedantic -Wextra # -Werror
                    $<$<CXX_COMPILER_ID:Clang>:-std=libc++> # $<$<CXX_COMPILER_ID:GNU>:>
    )
  elseif(MSVC)
    target_compile_options(binary PUBLIC /W4 /WX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add BinaryTests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(binary PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(binary PUBLIC -fprofile-arcs -ftest-coverage)
endif()
