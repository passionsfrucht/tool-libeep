libeep_headers = eep/eepmisc.h \
                 eep/eepraw.h \
                 eep/eepmem.h \
                 eep/eepio.h \
                 eep/var_string.h \
                 eep/val.h \
                 eep/version.h \
                 eep/winsafe.h
libavr_headers = avr/avr.h \
                 avr/avrcfg.h
libcnt_headers = cnt/cnt.h \
                 cnt/cntutils.h \
                 cnt/cnt_private.h \
                 cnt/cnt_version.h \
                 cnt/raw3.h \
                 cnt/trg.h \
                 cnt/rej.h \
                 cnt/riff.h
libeep_sources = libeep/eepmisc.c \
                 libeep/eepraw.c \
                 libeep/eepmem.c \
                 libeep/eepio.c \
                 libeep/var_string.c \
                 libeep/val.c
libavr_sources = libavr/avr.c \
                 libavr/avrcfg.c
libcnt_sources = libcnt/cnt.c \
                 libcnt/cntutils.c \
                 libcnt/trg.c \
                 libcnt/raw3.c \
                 libcnt/rej.c \
                 libcnt/riff.c

libeep_includedir=$(includedir)/$(PACKAGE)-$(LIBEEP_VERSION)
nobase_libeep_include_HEADERS = ${libeep_headers} ${libavr_headers} ${libcnt_headers}
noinst_HEADERS = ${libeep_private_headers}

lib_LTLIBRARIES = libeep.la

libeep_private_headers =
libeep_la_LIBADD =
libeep_la_LDFLAGS = -version-info $(PACKAGE_LIBRARY_VERSION)
libeep_la_SOURCES = ${libeep_sources} ${libavr_sources} ${libcnt_sources}
