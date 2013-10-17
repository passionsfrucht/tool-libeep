libeep_headers = eep/eepmisc.h \
                 eep/eepio.h \
                 eep/eepmem.h \
                 eep/eepraw.h \
                 eep/inttypes.h \
                 eep/stdint.h \
                 eep/val.h \
                 eep/var_string.h \
                 eep/winsafe.h \
                 v4/eep.h
libavr_headers = avr/avr.h \
                 avr/avrcfg.h
libcnt_headers = cnt/cnt.h \
                 cnt/cnt_private.h \
                 cnt/cntutils.h \
                 cnt/cnt_version.h \
                 cnt/raw3.h \
                 cnt/rej.h \
                 cnt/riff64.h \
                 cnt/riff.h \
                 cnt/trg.h
libeep_sources = libeep/eepmisc.c \
                 libeep/eepraw.c \
                 libeep/eepmem.c \
                 libeep/eepio.c \
                 libeep/var_string.c \
                 libeep/val.c \
                 v4/eep.c
libavr_sources = libavr/avr.c \
                 libavr/avrcfg.c
libcnt_sources = libcnt/cnt.c \
                 libcnt/cntutils.c \
                 libcnt/raw3.c \
                 libcnt/rej.c \
                 libcnt/riff64.c \
                 libcnt/riff.c \
                 libcnt/trg.c

libeep_includedir=$(includedir)/$(PACKAGE)-$(LIBEEP_VERSION)
nobase_libeep_include_HEADERS = ${libeep_headers} ${libavr_headers} ${libcnt_headers}
noinst_HEADERS = ${libeep_private_headers}

lib_LTLIBRARIES = libeep.la

# libeep_private_headers =
libeep_la_LIBADD = -lm
libeep_la_LDFLAGS = -version-info $(PACKAGE_LIBRARY_VERSION)
libeep_la_SOURCES = ${libeep_sources} ${libavr_sources} ${libcnt_sources}
