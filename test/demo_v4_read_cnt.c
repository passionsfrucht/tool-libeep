// system
#include <stdio.h>
#include <stdlib.h>
// libeep
#include <v4/eep.h>
///////////////////////////////////////////////////////////////////////////////
void
handle_file(const char *filename) {
  int handle, c, chanc;
  long s, sampc;

  handle = libeep_read(filename);
  if(handle == -1) {
    fprintf(stderr, "error opening %s", filename);
  }

  chanc = libeep_get_channel_count(handle);
  sampc = libeep_get_sample_count(handle);
  for(s=0;s<sampc;++s) {
    float * sample = libeep_get_samples(handle, s, s+1);
    printf("sample[%5i]:", s);
    for(c=0;c<chanc;++c) {
      printf(" %f", sample[c]);
    }
    free(sample);
    printf("\n");
  }
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
