include(ExternalProject)
if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    # when using the makefile generator, use the special variable $(MAKE) to invoke make
    # this enables the jobserver to work correctly
    set(MAKE_COMMAND "$(MAKE)")
else ()
    # invoke make explicitly
    # in this case, we assume the parent build system is running in parallel already so no -j flag is added
    find_program(MAKE_COMMAND NAMES make gmake)
endif ()

if (INSTALL_DEPENDENCIES)
    set(HTSLIB_INSTALL ${MAKE_COMMAND} install prefix=${CMAKE_INSTALL_PREFIX})
else ()
    set(HTSLIB_INSTALL "")
endif ()


# build htslib
set(htslib_PREFIX ${CMAKE_BINARY_DIR}/htslib)

externalproject_add(
        htslib
        BUILD_IN_SOURCE 1
        URL https://github.com/samtools/htslib/releases/download/1.15.1/htslib-1.15.1.tar.bz2
        PREFIX ${htslib_PREFIX}
        CONFIGURE_COMMAND autoconf && ./configure --prefix=${CMAKE_BINARY_DIR}/htslib --disable-lzma --disable-s3 --disable-plugins --disable-bz2 --disable-libcurl
        BUILD_COMMAND make CFLAGS=${CMAKE_C_FLAGS}
        INSTALL_COMMAND "${HTSLIB_INSTALL}"
#        LOG_DOWNLOAD 0
#        LOG_UPDATE 0
#        LOG_CONFIGURE 0
#        LOG_BUILD 0
#        LOG_TEST 0
#        LOG_INSTALL 0
)

externalproject_get_property(htslib SOURCE_DIR)
set(HTSLIB_SRC_DIR ${SOURCE_DIR})
include_directories("${HTSLIB_SRC_DIR}/htslib")
