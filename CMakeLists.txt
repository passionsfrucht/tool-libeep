cmake_minimum_required(VERSION 2.8)

set(LIBEEP_VERSION "3.3.171")
set(prefix ${CMAKE_INSTALL_PREFIX})
set(PACKAGE libeep)

project(libeep-${LIBEEP_VERSION})

add_definitions(-DLIBEEP_VERSION="${LIBEEP_VERSION}")
if(UNIX)
  add_definitions(-fPIC)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  configure_file(libeep.pc.in ${CMAKE_BINARY_DIR}/libeep.pc)
  install(FILES ${CMAKE_BINARY_DIR}/libeep.pc DESTINATION lib/pkgconfig)
endif()
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DNOMINMAX)
  add_definitions(-DSTRICT)
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DVC_EXTRALEAN)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  set(Eep_def windows/Eep.def)
endif(WIN32)

set(Eep_sources
  ${CMAKE_SOURCE_DIR}/src/libavr/avr.c
  ${CMAKE_SOURCE_DIR}/src/libavr/avrcfg.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/cnt.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/cntutils.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/raw3.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/rej.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/riff64.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/riff.c
  ${CMAKE_SOURCE_DIR}/src/libcnt/trg.c
  ${CMAKE_SOURCE_DIR}/src/libeep/eepio.c
  ${CMAKE_SOURCE_DIR}/src/libeep/eepmem.c
  ${CMAKE_SOURCE_DIR}/src/libeep/eepmisc.c
  ${CMAKE_SOURCE_DIR}/src/libeep/eepraw.c
  ${CMAKE_SOURCE_DIR}/src/libeep/val.c
  ${CMAKE_SOURCE_DIR}/src/libeep/var_string.c
)
add_library(Eep SHARED
  src/v4/eep.c
  ${Eep_sources}
  ${Eep_def}
)
include_directories(src)
include_directories(${CMAKE_BINARY_DIR}/gen/include)

install(TARGETS Eep DESTINATION lib)

install(FILES src/v4/eep.h DESTINATION include/${CMAKE_PROJECT_NAME}/v4)

########
# java #
########
if(NOT EXISTS $ENV{JAVA_HOME})
  message("please set the environment variable JAVA_HOME to build the JNI package")
else()
  add_subdirectory(java)
endif()

##########
# matlab #
##########
if(IS_DIRECTORY $ENV{MATLAB})
  if(IS_DIRECTORY $ENV{MATLAB}/bin/win64)
    # windows 64
    set(MATLAB_BIN $ENV{MATLAB}/bin/win64)
    set(MATLAB_EXT mexw64)
    set(MATLAB_MEX_CFLAGS -DMEX_CXXFLAGS=-ansi -DWIN32)
  elseif(IS_DIRECTORY $ENV{MATLAB}/bin/win32)
    # windows 32
    set(MATLAB_BIN $ENV{MATLAB}/bin/win32)
    set(MATLAB_EXT mexw32)
    set(MATLAB_MEX_CFLAGS -DMEX_CXXFLAGS=-ansi -DWIN32)
  elseif(IS_DIRECTORY $ENV{MATLAB}/bin/glnx32)
    # linux 32
    set(MATLAB_BIN $ENV{MATLAB}/bin/glnx32)
    set(MATLAB_EXT mexa32)
  elseif(IS_DIRECTORY $ENV{MATLAB}/bin/glnxa64)
    # linux 64
    set(MATLAB_BIN $ENV{MATLAB}/bin/glnxa64)
    set(MATLAB_EXT mexa64)
    # set(MATLAB_MEX_CFLAGS "CFLAGS=-std=c99 -fPIC -DLIBEEP_VERSION=\"${LIBEEP_VERSION}-MATLAB\"")
    set(MATLAB_MEX_CFLAGS "CFLAGS=-std=c99 -fPIC")
  else()
    # fallback, mac 64
    set(MATLAB_BIN $ENV{MATLAB}/bin)
    set(MATLAB_EXT mexmaci64)
  endif()

  # if(WIN32)
  #   SET(CMAKE_FIND_LIBRARY_PREFIXES "")
  #   SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".lib")
  # endif()

# find_library(MATLAB_MEX_LIBRARY mex ${MATLAB_BIN} NO_DEFAULT_PATH)
# find_library(MATLAB_MX_LIBRARY  mx  ${MATLAB_BIN} NO_DEFAULT_PATH)
# find_library(MATLAB_ENG_LIBRARY eng ${MATLAB_BIN} NO_DEFAULT_PATH)

  message("Eep................. ${Eep}")
  message("MATLAB_BIN.......... ${MATLAB_BIN}")
  message("MATLAB_EXT.......... ${MATLAB_EXT}")
  message("CMAKE_BINARY_DIR.... ${CMAKE_BINARY_DIR}")
# message("MATLAB_MEX_LIBRARY.. ${MATLAB_MEX_LIBRARY}")
# message("MATLAB_ENG_LIBRARY.. ${MATLAB_ENG_LIBRARY}")
# message("MATLAB_MX_LIBRARY... ${MATLAB_MX_LIBRARY}")

  foreach(matlab_target read_eep_avr read_eep_cnt read_eep_trg write_eep_avr write_eep_cnt)
    add_custom_command(OUTPUT ${matlab_target}.${MATLAB_EXT}
                       COMMAND ${MATLAB_BIN}/mex
                       ARGS ${CMAKE_SOURCE_DIR}/mex/matlab/${matlab_target}.c -I${CMAKE_SOURCE_DIR}/src -L${CMAKE_BINARY_DIR}/debug
                            ${Eep_sources}
                            ${MATLAB_MEX_CFLAGS}
                       DEPENDS ${CMAKE_SOURCE_DIR}/mex/matlab/${matlab_target}.c ${Eep_sources}
                      )
    # target_link_libraries(${matlab_target}.${MATLAB_EXT} Eep)
    add_custom_target(${matlab_target} ALL DEPENDS ${matlab_target}.${MATLAB_EXT})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${matlab_target}.${MATLAB_EXT} DESTINATION share/libeep/matlab)
  endforeach()
  # add_library(read_eep_cnt MODULE ${CMAKE_SOURCE_DIR}/mex/matlab/read_eep_cnt.c)
  # target_link_libraries(read_eep_cnt Eep)
  # set_target_properties(read_eep_cnt PROPERTIES PREFIX "" SUFFIX  ".${MATLAB_EXT}" CMAKE_C_COMPILER "${MATLAB_BIN}/mex")
  message("please set the environment variable MATLAB to a matlab directry to build the MATLAB plugin")
endif()

if(UNIX)
  find_library(Math m)
  target_link_libraries(Eep ${Math})
endif(UNIX)

add_executable(demo_read_avr test/demo_read_avr.c)
add_executable(demo_read_cnt test/demo_read_cnt.c)
add_executable(demo_write_avr test/demo_write_avr.c)
add_executable(demo_write_cnt test/demo_write_cnt.c)
target_link_libraries(demo_read_avr Eep ${MATH})
target_link_libraries(demo_read_cnt Eep ${MATH})
target_link_libraries(demo_write_avr Eep ${MATH})
target_link_libraries(demo_write_cnt Eep ${MATH})
