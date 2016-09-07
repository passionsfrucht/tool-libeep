cmake_minimum_required(VERSION 2.8)

set(LIBEEP_VERSION_MAJOR 3)
set(LIBEEP_VERSION_MINOR 3)
set(LIBEEP_VERSION_PATCH 175)
set(LIBEEP_VERSION "${LIBEEP_VERSION_MAJOR}.${LIBEEP_VERSION_MINOR}.${LIBEEP_VERSION_PATCH}")
set(prefix ${CMAKE_INSTALL_PREFIX})
set(PACKAGE libeep)

project(libeep-${LIBEEP_VERSION})

add_definitions(-DLIBEEP_VERSION="${LIBEEP_VERSION}")
if(UNIX)
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
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

  configure_file(
    installer.nsi.in
    ${CMAKE_BINARY_DIR}/installer.nsi
  )
endif(WIN32)

set(Eep_sources
  src/libavr/avr.c
  src/libavr/avrcfg.c
  src/libcnt/evt.c
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
)
add_library(EepObjects OBJECT
  ${Eep_sources}
  src/v4/eep.c
)
add_library(EepStatic STATIC
  $<TARGET_OBJECTS:EepObjects>
)
add_library(Eep SHARED
  $<TARGET_OBJECTS:EepObjects>
  ${Eep_def}
)
# set_target_properties(Eep PROPERTIES VERSION ${LIBEEP_VERSION_MAJOR}.${LIBEEP_VERSION_MINOR}.${LIBEEP_VERSION_PATCH} SOVERSION ${LIBEEP_VERSION_MAJOR})
include_directories(src)
include_directories(${CMAKE_BINARY_DIR}/gen/include)

install(TARGETS Eep DESTINATION lib)

install(FILES src/v4/eep.h DESTINATION include/libeep-${LIBEEP_VERSION}/v4)

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
    set(MATLAB_MEX_CFLAGS "CFLAGS=-std=c99 -fPIC")
  else()
    # fallback, mac 64
    set(MATLAB_BIN $ENV{MATLAB}/bin)
    set(MATLAB_EXT mexmaci64)
  endif()

  message("Eep................. ${Eep}")
  message("MATLAB_BIN.......... ${MATLAB_BIN}")
  message("MATLAB_EXT.......... ${MATLAB_EXT}")
  message("CMAKE_BINARY_DIR.... ${CMAKE_BINARY_DIR}")

  foreach(eep_source_file ${Eep_sources})
    get_filename_component(eep_source_file_abs ${eep_source_file} ABSOLUTE)
    set(Eep_sources_abs ${Eep_sources_abs} ${eep_source_file_abs})
  endforeach()


  foreach(matlab_target read_eep_avr read_eep_cnt read_eep_trg write_eep_avr write_eep_cnt get_eep_version)
    add_custom_command(OUTPUT ${matlab_target}.${MATLAB_EXT}
                       COMMAND ${MATLAB_BIN}/mex
                       ARGS ${CMAKE_SOURCE_DIR}/mex/matlab/${matlab_target}.c -I${CMAKE_SOURCE_DIR}/src -L${CMAKE_BINARY_DIR}/debug
                            ${Eep_sources_abs}
                            ${MATLAB_MEX_CFLAGS}
                            -DLIBEEP_VERSION_MAJOR=${LIBEEP_VERSION_MAJOR}
                            -DLIBEEP_VERSION_MINOR=${LIBEEP_VERSION_MINOR}
                            -DLIBEEP_VERSION_PATCH=${LIBEEP_VERSION_PATCH}
                       DEPENDS ${CMAKE_SOURCE_DIR}/mex/matlab/${matlab_target}.c ${Eep_sources}
                      )
    # target_link_libraries(${matlab_target}.${MATLAB_EXT} Eep)
    add_custom_target(${matlab_target} ALL DEPENDS ${matlab_target}.${MATLAB_EXT})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${matlab_target}.${MATLAB_EXT} DESTINATION share/libeep/matlab)
    install(FILES ${CMAKE_SOURCE_DIR}/mex/matlab/${matlab_target}.m DESTINATION share/libeep/matlab)
  endforeach()
elseif()
  message("please set the environment variable MATLAB to a matlab directry to build the MATLAB plugin")
endif()

##########
# python #
##########
find_package(PythonLibs 2.7)
if(PYTHONLIBS_FOUND)
  message("python libs: ${PYTHON_LIBRARIES}")
  add_library(pyeep MODULE python/pyeep.c)
  target_include_directories(pyeep PRIVATE ${PYTHON_INCLUDE_DIRS})
  target_link_libraries(pyeep PRIVATE EepStatic ${MATH} ${PYTHON_LIBRARIES})
  install(FILES python/__init__.py DESTINATION share/libeep/python/libeep)
  install(TARGETS pyeep DESTINATION share/libeep/python/libeep)

  if(UNIX)
    set_target_properties(pyeep PROPERTIES PREFIX "")
  endif()
  if(WIN32)
    set_target_properties(pyeep PROPERTIES SUFFIX .pyd)
  endif()
endif()

if(UNIX)
  find_library(Math m)
  target_link_libraries(Eep ${Math})
endif(UNIX)

add_executable(demo_read_avr test/demo_read_avr.c)
add_executable(demo_read_cnt test/demo_read_cnt.c)
add_executable(demo_read_evt  test/demo_read_evt.c)
add_executable(demo_v4_read_cnt test/demo_v4_read_cnt.c)
add_executable(demo_v4_write_cnt test/demo_v4_write_cnt.c)
add_executable(demo_write_avr test/demo_write_avr.c)
add_executable(demo_write_cnt test/demo_write_cnt.c)
add_executable(libeep_info test/libeep_info.c)
target_link_libraries(demo_read_avr Eep ${MATH})
target_link_libraries(demo_read_cnt Eep ${MATH})
target_link_libraries(demo_read_evt Eep ${MATH})
target_link_libraries(demo_v4_read_cnt Eep ${MATH})
target_link_libraries(demo_v4_write_cnt Eep ${MATH})
target_link_libraries(demo_write_avr Eep ${MATH})
target_link_libraries(demo_write_cnt Eep ${MATH})
target_link_libraries(libeep_info Eep)
