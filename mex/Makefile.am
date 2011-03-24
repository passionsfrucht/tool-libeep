
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
  matlab/read_eep_cnt.m \
  matlab/read_eep_avr.m \
  matlab/read_eep_trg.m \
  matlab/read_eep_trial.m \
  matlab/read_eep_rej.m \
  matlab/write_eep_cnt.m
matlabdir = ${datadir}/matlab
dist_matlab_DATA = \
  ${matlab_files}
if CONDMEX
matlab_DATA = ${matlab_files}
matlab_DATA += \
  matlab/read_eep_cnt.${MEXEXT} \
  matlab/read_eep_avr.${MEXEXT} \
  matlab/write_eep_cnt.${MEXEXT}
endif

###############
# eeglab part #
###############
eeglab_files = \
  eeglab/eegplugin_eepimport.m \
  eeglab/loadeep_avg.m \
  eeglab/loadeep.m \
  eeglab/pop_loadeep_avg.m \
  eeglab/pop_loadeep.m
eeglabdir = ${datadir}/eeglab
dist_eeglab_DATA = ${eeglab_files}
if CONDMEX
eeglab_DATA = ${eeglab_files}
endif
