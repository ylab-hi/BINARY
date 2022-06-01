cmake_minimum_required(VERSION 3.15)

project(
        Sv2nl
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

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")

    if(UNIX)
        add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")    #this will allow to use same _DEBUG macro available in both Linux as well as Windows - MSCV environment. Easy to put Debug specific code.
    endif(UNIX)

endif()
message(STATUS "Started CMake for  ${PROJECT_NAME} v${PROJECT_VERSION}...")
message(STATUS "Compiler name: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Cmake Binary path: ${CMAKE_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(CPM)
include(functions)
find_package(Threads REQUIRED)


# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

cpmaddpackage(
    NAME spdlog
    GITHUB_REPOSITORY  gabime/spdlog
    VERSION 1.10.0
    OPTIONS "SPDLOG_INSTALL YES"
)


include(htslib)

# ---- Add source files ----

add_library(${PROJECT_NAME}
    include/sv2nl/csv.h
    include/sv2nl/output_container.h
    include/sv2nl/test.h
    include/sv2nl/utils.h
    include/sv2nl/vcf_reader.h
    # source
    source/vcf_reader.cpp)

add_dependencies(${PROJECT_NAME} htslib)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog ${htslib_LIB})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${dbg-macro_SOURCE_DIR})


set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_EXTENSIONS OFF)


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
    INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
    INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
    VERSION_HEADER "${VERSION_HEADER_LOCATION}"
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES "spdlog 1.10.0;htslib 1.15.1"
)
