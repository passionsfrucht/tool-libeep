cmake_minimum_required(VERSION 2.8)

set(LIBEEP_VERSION "3.3.169")

project(libeep-${LIBEEP_VERSION})

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
  ${Eep_def}
)
configure_file(
  src/eep/version.h.in
  ${CMAKE_BINARY_DIR}/gen/include/eep/version.h
)
include_directories(src)

install(TARGETS Eep DESTINATION lib)

install(FILES ${CMAKE_BINARY_DIR}/gen/include/eep/version.h DESTINATION include/${CMAKE_PROJECT_NAME}/eep)
foreach(f avr.h avrcfg.h)
  install(FILES src/avr/${f} DESTINATION include/${CMAKE_PROJECT_NAME}/avr)
endforeach(f)
foreach(f eepmisc.h eepraw.h eepmem.h eepio.h var_string.h val.h winsafe.h)
  install(FILES src/eep/${f} DESTINATION include/${CMAKE_PROJECT_NAME}/eep)
endforeach(f)
foreach(f cnt.h cntutils.h cnt_private.h cnt_version.h raw3.h trg.h rej.h riff.h)
  install(FILES src/cnt/${f} DESTINATION include/${CMAKE_PROJECT_NAME}/cnt)
endforeach(f)
foreach(f LICENSE LICENSE.addendum)
  install(FILES ${f} DESTINATION share/${CMAKE_PROJECT_NAME})
endforeach(f)
