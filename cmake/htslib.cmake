include(ExternalProject)

set(htslib_PREFIX ${CMAKE_BINARY_DIR}/cmake-ext/htslib-prefix)
set(htslib_INSTALL ${CMAKE_BINARY_DIR}/cmake-ext/htslib-install)

if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    set(MAKE_COMMAND "$(MAKE)")
else()
    find_program(MAKE_COMMAND NAMES make gmake)
endif()

message(STATUS "Building htslib ${MAKE_COMMAND}")
ExternalProject_Add(htslib
        PREFIX ${htslib_PREFIX}
        URL https://github.com/samtools/htslib/releases/download/1.15.1/htslib-1.15.1.tar.bz2
        BUILD_IN_SOURCE 1
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND autoconf && ./configure --prefix=${htslib_PREFIX}  --disable-bz2 --disable-lzma --disable-gcs --disable-s3 --disable-plugins --disable-libcurl
        BUILD_COMMAND ${MAKE_COMMAND} lib-static
        INSTALL_COMMAND ${MAKE_COMMAND} install prefix=${htslib_INSTALL}
        )

include(zlib)
add_dependencies(htslib zlib)
include_directories(${htslib_INSTALL}/include/htslib)
set(htslib_LIB ${htslib_INSTALL}/lib/libhts.a
    ${zlib_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
    )
