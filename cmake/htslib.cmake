include(ExternalProject)


set(htslib_PREFIX ${CMAKE_BINARY_DIR}/cmake-ext/htslib-prefix)
set(htslib_INSTALL ${CMAKE_BINARY_DIR}/cmake-ext/htslib-install)

if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    set(MAKE_COMMAND "$(MAKE)")
else()
    find_program(MAKE_COMMAND NAMES make gmake)
endif()

# zlib
find_package(ZLIB)
MESSAGE(STATUS "ZLIB FOUNDED: ${ZLIB_FOUND}")
if(ZLIB_FOUND)
    set(deps_LIB z)
else()
    include(../cmake/zlib.cmake)
    add_dependencies(htslib zlib)
endif()


# lzma
find_package (LibLZMA)
if (LIBLZMA_FOUND)
    include_directories(SYSTEM ${LIBLZMA_INCLUDE_DIRS})
    list(APPEND deps_LIB ${LIBLZMA_LIBRARIES})
endif ()

# config cmake files path
# deflate
list(APPEND CMAKE_MODULE_PATH ../cmake)
find_package(Deflate)
message(STATUS "Deflate FOUND: ${Deflate_FOUND}")
if(Deflate_FOUND)
    include_directories(SYSTEM ${Deflate_INCLUDE_DIRS})
    list(APPEND deps_LIB ${Deflate_LIBRARIES})
endif()

message(STATUS "Building htslib ${MAKE_COMMAND}")
set(flags "-O2 -g -fPIC")
set(disable_flags "--disable-bz2 --disable-lzma --disable-gcs --disable-s3 --disable-plugins --disable-libcurl")
ExternalProject_Add(htslib
        PREFIX ${htslib_PREFIX}
        URL https://github.com/samtools/htslib/releases/download/1.15.1/htslib-1.15.1.tar.bz2
        BUILD_IN_SOURCE 1
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND autoreconf -i && ./configure --prefix=${htslib_PREFIX} ${disable_flags}
        BUILD_COMMAND ${MAKE_COMMAND} CFLAGS=${flags} lib-static
        INSTALL_COMMAND ${MAKE_COMMAND} install prefix=${htslib_INSTALL}
        )

include_directories(${htslib_INSTALL}/include/htslib)
set(htslib_LIB ${htslib_INSTALL}/lib/libhts.a
    ${deps_LIB}
    )
