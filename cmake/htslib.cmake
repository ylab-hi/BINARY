include(ExternalProject)

# find htslib
find_package(HTSlib)
if(HTSlib_FOUND)
  message(STATUS "HTSlib_USE_STATIC_LIBS: ${HTSlib_USE_STATIC_LIBS}")

  # not found, try to build it to static libs
else()
  set(htslib_PREFIX ${CMAKE_BINARY_DIR}/cmake-ext/htslib-prefix)
  set(htslib_INSTALL ${CMAKE_BINARY_DIR}/cmake-ext/htslib-install)

  if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    set(MAKE_COMMAND "$(MAKE)")
  else()
    find_program(MAKE_COMMAND NAMES make gmake)
  endif()

  message(STATUS "Building static htslib ${MAKE_COMMAND}")
  set(flags "-O2 -g -fPIC")
  set(disable_flags --disable-gcs --disable-s3 --disable-plugins)

  # find lzma
  find_package(LibLZMA)
  if(LIBLZMA_FOUND)
    include_directories(SYSTEM ${LIBLZMA_INCLUDE_DIRS})
    list(APPEND deps_LIB ${LIBLZMA_LIBRARIES})
  else()
    list(APPEND disable_flags --disable-lzma)
  endif()

  find_package(CURL)
  if(CURL_FOUND)
    include_directories(SYSTEM ${CURL_INCLUDE_DIRS})
    list(APPEND deps_LIB ${CURL_LIBRARIES})
  else()
    list(APPEND disable_flags --disable-libcurl)
  endif()

  find_package(BZip2)
  if(BZip2_FOUND)
    include_directories(SYSTEM ${BZip2_INCLUDE_DIRS})
    list(APPEND deps_LIB ${BZip2_LIBRARIES})
  else()
    list(APPEND disable_flags --disable-bz2)
  endif()

  # config cmake files path deflate
  find_package(Deflate)
  if(Deflate_FOUND)
    include_directories(SYSTEM ${Deflate_INCLUDE_DIRS})
    list(APPEND deps_LIB ${Deflate_LIBRARIES})
  endif()

  message(STATUS " dependencies: ${deps_LIB}")

  ExternalProject_Add(
    htslib
    PREFIX ${htslib_PREFIX}
    URL https://github.com/samtools/htslib/releases/download/1.15.1/htslib-1.15.1.tar.bz2
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND autoreconf -i && ./configure --prefix=${htslib_PREFIX} ${disable_flags}
    BUILD_COMMAND ${MAKE_COMMAND} CFLAGS=${flags} lib-static
    INSTALL_COMMAND ${MAKE_COMMAND} install prefix=${htslib_INSTALL}
  )

  message(STATUS "ZLIB_BUILD: ${ZLIB_BUILD}")
  if(ZLIB_BUILD)
    include(../cmake/zlib.cmake)
    add_dependencies(htslib zlib)
  else()
    find_package(ZLIB)
    if(ZLIB_FOUND)
      include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
      list(APPEND deps_LIB ${ZLIB_LIBRARIES})
    else()
      # build zlib from source
      message(STATUS "Building zlib from source")
      include(../cmake/zlib.cmake)
      add_dependencies(htslib zlib)
      list(APPEND deps_LIB ${zlib_LIBRARIES})
      endif()
  endif()
  list(APPEND deps_LIB ${zlib_LIBRARIES})

  set(HTSlib_INCLUDE_DIRS ${htslib_INSTALL}/include)
  set(HTSlib_LIBRARIES ${htslib_INSTALL}/lib/libhts.a ${deps_LIB})
  message(STATUS "HTSlib_INCLUDE_DIRS: ${HTSlib_INCLUDE_DIRS}")
  message(STATUS "HTSlib_LIBRARIES: ${HTSlib_LIBRARIES}")

endif()
