cmake_minimum_required(VERSION 2.8)

set(LIBEEP_VERSION "3.3.171")
set(prefix ${CMAKE_INSTALL_PREFIX})
set(PACKAGE libeep)

project(libeep-${LIBEEP_VERSION})

add_definitions(-DLIBEEP_VERSION="${LIBEEP_VERSION}")
if(UNIX)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  configure_file(libeep.pc.in ${CMAKE_BINARY_DIR}/libeep.pc)
  install(FILES ${CMAKE_BINARY_DIR}/libeep.pc DESTINATION lib/pkgconfig)
endif()
if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
  set(Eep_def windows/Eep.def)
endif(WIN32)

add_library(Eep SHARED
  src/libavr/avr.c
  src/libavr/avrcfg.c
  src/libcnt/cnt.c
  src/libcnt/cntutils.c
  src/libcnt/raw3.c
  src/libcnt/rej.c
  src/libcnt/riff64.c
  src/libcnt/riff.c
  src/libcnt/trg.c
  src/libeep/eepio.c
  src/libeep/eepmem.c
  src/libeep/eepmisc.c
  src/libeep/eepraw.c
  src/libeep/val.c
  src/libeep/var_string.c
  src/v4/eep.c
  ${Eep_def}
)
include_directories(src)
include_directories(${CMAKE_BINARY_DIR}/gen/include)

install(TARGETS Eep DESTINATION lib)

install(FILES src/v4/eep.h DESTINATION include/${CMAKE_PROJECT_NAME}/v4)

if(NOT EXISTS $ENV{JAVA_HOME})
  message("please set the environment variable JAVA_HOME to build the JNI package")
else()
  add_subdirectory(java)
endif()

if(UNIX)
  find_library(Math m)
  target_link_libraries(Eep ${Math})
endif(UNIX)

add_executable(demo_read_avr test/demo_read_avr.c)
add_executable(demo_read_cnt test/demo_read_cnt.c)
add_executable(demo_write_avr test/demo_write_avr.c)
add_executable(demo_write_cnt test/demo_write_cnt.c)
target_link_libraries(demo_read_avr Eep)
target_link_libraries(demo_read_cnt Eep)
target_link_libraries(demo_write_avr Eep)
target_link_libraries(demo_write_cnt Eep)
