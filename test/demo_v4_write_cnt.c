// system
#include <stdio.h>
#include <stdlib.h>
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

  handle = libeep_write_cnt(filename, 512, CHANNEL_COUNT, channel_names, channel_units);
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
