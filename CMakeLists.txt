cmake_minimum_required(VERSION 3.15..3.22)

project(
        "sv2nl"
        VERSION 1.0
        LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "Started CMake for  ${PROJECT_NAME} v${PROJECT_VERSION}...\n")
message(STATUS "Compiler name: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Cmake Binary path: ${CMAKE_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(CPM)
find_package(Threads REQUIRED)

# cpmaddpackage("gh:sharkdp/dbg-macro@0.4.0")
cpmaddpackage("gh:gabime/spdlog@1.10.0")
#cpmaddpackage("gh:ericniebler/range-v3#0.11.0")
cpmaddpackage("gh:p-ranav/argparse@2.4")
cpmaddpackage(
        NAME thread-pool
        GITHUB_REPOSITORY DeveloperPaul123/thread-pool
        GIT_TAG 0.5.0 #0cea9c12fb30cb677696c0dce6228594ce26171a change this to latest commit or release tag
        OPTIONS "TP_BUILD_EXAMPLES OFF" "TP_BUILD_BENCHMARKS OFF" "TP_BUILD_TESTS OFF"
)
include(htslib)


if(UNIX)
    add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")    #this will allow to use same _DEBUG macro available in both Linux as well as Windows - MSCV environment. Easy to put Debug specific code.
endif(UNIX)


if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()


# ---- Add source files ----

# Note: globbing sources is considered bad practice as include(ExternalProjectinclude(ExternalProjectinclude(ExternalProject)))CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")


function(target_link_libraries_system target)
    set(options PRIVATE PUBLIC INTERFACE)
    cmake_parse_arguments(TLLS "${options}" "" "" ${ARGN})
    foreach(op ${options})
        if(TLLS_${op})
            set(scope ${op})
        endif()
    endforeach(op)
    set(libs ${TLLS_UNPARSED_ARGUMENTS})

    foreach(lib ${libs})
        get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
        if(lib_include_dirs)
            if(scope)
                target_include_directories(${target} SYSTEM ${scope} ${lib_include_dirs})
            else()
                target_include_directories(${target} SYSTEM PRIVATE ${lib_include_dirs})
            endif()
        else()
            message("Warning: ${lib} doesn't set INTERFACE_INCLUDE_DIRECTORIES. No include_directories set.")
        endif()
        if(scope)
            target_link_libraries(${target} ${scope} ${lib})
        else()
            target_link_libraries(${target} ${lib})
        endif()
    endforeach()
endfunction(target_link_libraries_system)

add_executable(${PROJECT_NAME} src/main.cpp include/csv.h include/output_container.h src/vcf_reader.cpp include/vcf_reader.h)
add_dependencies(${PROJECT_NAME} htslib)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wnon-virtual-dtor -pedantic)
target_link_libraries_system(${PROJECT_NAME} PRIVATE argparse) # suppress unused parameters warning
target_link_libraries(${PROJECT_NAME} PRIVATE dp::thread-pool spdlog ${htslib_LIB})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${dbg-macro_SOURCE_DIR})


set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        CXX_EXTENSIONS OFF)
