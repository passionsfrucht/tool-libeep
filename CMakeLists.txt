cmake_minimum_required(VERSION 2.8)

set(LIBEEP_VERSION "3.3.169")

project(libeep-${LIBEEP_VERSION})

if(UNIX)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if(WIN32)
  set(Eep_def windows/Eep.def)
endif(WIN32)
add_library(Eep SHARED
  src/libavr/avr.c
  src/libavr/avrcfg.c
  src/libcnt/cntutils.c
  src/libcnt/cnt.c
  src/libcnt/raw3.c
  src/libcnt/rej.c
  src/libcnt/riff.c
  src/libcnt/trg.c
  src/libeep/eepmisc.c
  src/libeep/eepio.c
  src/libeep/eepmem.c
  src/libeep/var_string.c
  src/libeep/eepraw.c
  src/libeep/val.c
  src/v4/eep.c
  ${Eep_def}
)
configure_file(
  src/eep/version.h.in
  ${CMAKE_BINARY_DIR}/gen/include/eep/version.h
)
include_directories(src)
include_directories(${CMAKE_BINARY_DIR}/gen/include)

install(TARGETS Eep DESTINATION lib)

install(FILES src/v4/eep.h DESTINATION include/${CMAKE_PROJECT_NAME}/v4)

if(NOT EXISTS "$ENV{JAVA_HOME}")
  message("please set the environment variable JAVA_HOME to build the JNI package")
else()
  add_subdirectory(java)
endif()
