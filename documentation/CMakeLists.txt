cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(BinaryDocs)

message(STATUS "Building BinaryDocs")

# ---- Dependencies ----

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

include(CPM)

# look for Doxygen package
find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

CPMAddPackage(NAME binary SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../library)

# ---- Doxygen variables ----

# set Doxyfile variables
set(DOXYGEN_PROJECT_NAME binary)
set(DOXYGEN_PROJECT_VERSION ${BINARY_VERSION})
set(DOXYGEN_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../library")
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

# set sphnix variables
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)

configure_file(${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_target(
  DoxygenTarget
  ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUTPUT_DIRECTORY}/../Doxyfile
  COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_target(
  BinaryDocs ALL
  COMMAND ${SPHINX_EXECUTABLE} -b html ${SPHINX_SOURCE} ${SPHINX_BUILD}
          -Dbreathe_projects.binary=${DOXYGEN_OUTPUT_DIRECTORY}/xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating documentation with Sphinx"
  DEPENDS DoxygenTarget
)

# ---- Install ----
