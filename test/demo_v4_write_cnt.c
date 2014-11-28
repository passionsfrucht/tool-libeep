// system
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// libeep
#include <v4/eep.h>
///////////////////////////////////////////////////////////////////////////////
#define CHANNEL_COUNT 10
const char * channel_names[CHANNEL_COUNT] = { "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9" };
const char * channel_units[CHANNEL_COUNT] = { "uV", "uV", "uV", "uV", "uV", "uV", "uV", "uV", "uV", "uV" };
///////////////////////////////////////////////////////////////////////////////
void
handle_file(const char *filename) {
  int handle, i, c;
  chaninfo_t channel_info_handle;
  recinfo_t  recording_info_handle;

  // setup channel information
  channel_info_handle = libeep_create_channel_info();
  for (i = 0; i < CHANNEL_COUNT; ++i) {
	  libeep_add_channel(channel_info_handle, channel_names[i], "ref", channel_units[i]);
  }

  // setup recording info
  recording_info_handle = libeep_create_recinfo();
  libeep_set_start_time(recording_info_handle, time(NULL));
  libeep_set_patient_handedness(recording_info_handle, 'R');
  libeep_set_patient_sex(recording_info_handle, 'M');
  libeep_set_patient_name(recording_info_handle, "John Doe");
  libeep_set_hospital(recording_info_handle, "Hospital");
  libeep_set_date_of_birth(recording_info_handle, 1950, 6, 28);

  handle = libeep_write_cnt(filename, 512, channel_info_handle, recording_info_handle, 1);
  if(handle == -1) {
    fprintf(stderr, "error opening %s", filename);
  }

  for(i=0;i<1024;++i) {
    float * samples = (float *)malloc(sizeof(float) * CHANNEL_COUNT);
    for(c=0;c<CHANNEL_COUNT;++c) {
      samples[c] = 7.90094; // (float)(i);
    }
    libeep_add_samples(handle, samples, 1);
  }
  libeep_close(handle);
}
///////////////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv) {
  libeep_init();

  int i;
  for(i=1;i<argc;i++) {
    handle_file(argv[i]);
  }

  libeep_exit();

  return 0;
}
