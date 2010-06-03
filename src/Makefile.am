libeep_headers = eep/eepmisc.h \
                 eep/eepraw.h \
                 eep/eepmem.h \
                 eep/eepio.h \
                 eep/var_string.h \
                 eep/val.h \
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

libeep_includedir=$(includedir)/$(PACKAGE)-$(VERSION)
nobase_libeep_include_HEADERS = ${libeep_headers} ${libavr_headers} ${libcnt_headers}
noinst_HEADERS = ${libeep_private_headers}

lib_LTLIBRARIES = libeep.la

libeep_private_headers =
libeep_la_LIBADD =
libeep_la_LDFLAGS = -version-info $(PACKAGE_LIBRARY_VERSION)
libeep_la_SOURCES = ${libeep_sources} ${libavr_sources} ${libcnt_sources}

#########
# other #
#########
SUFFIXES = .${MEXEXT}
%.${MEXEXT}:	%.c
				${MEX} ${MEXOPTS} ${MEXFLAGS} -I../src ${EXTRA_CFLAGS} ${EXTRA_LIBS} $< ${MEXLDADD} -o $@ -L../src/.libs -leep
mostlyclean-local:
					test -z '${MEXEXT}' || rm -f *.${MEXEXT}

###############
# matlab part #
###############
matlab_files = \
  ../mex/matlab/read_eep_cnt.m \
  ../mex/matlab/read_eep_avr.m \
  ../mex/matlab/read_eep_trg.m \
  ../mex/matlab/read_eep_trial.m \
  ../mex/matlab/read_eep_rej.m \
  ../mex/matlab/write_eep_cnt.m
matlabdir = ${datadir}/matlab
dist_matlab_DATA = \
  ${matlab_files} \
  ../mex/matlab/read_eep_avr.c \
  ../mex/matlab/read_eep_cnt.c \
  ../mex/matlab/read_eep_trg.c \
  ../mex/matlab/write_eep_cnt.c
if CONDMEX
matlab_DATA = ${matlab_files}
matlab_DATA += \
  ../mex/matlab/read_eep_cnt.${MEXEXT} \
  ../mex/matlab/read_eep_avr.${MEXEXT} \
  ../mex/matlab/write_eep_cnt.${MEXEXT}
endif

###############
# eeglab part #
###############
eeglab_files = \
  ../mex/eeglab/eegplugin_eepimport.m \
  ../mex/eeglab/loadeep_avg.m \
  ../mex/eeglab/loadeep.m \
  ../mex/eeglab/pop_loadeep_avg.m \
  ../mex/eeglab/pop_loadeep.m
eeglabdir = ${datadir}/eeglab
dist_eeglab_DATA = ${eeglab_files}
if CONDMEX
eeglab_DATA = ${eeglab_files}
endif
