// system
#include <stdio.h>
#include <stdlib.h>
// libeep
#include <v4/eep.h>
///////////////////////////////////////////////////////////////////////////////
void
handle_file(const char *filename) {
  int i, handle, c, chanc, triggerc;
  long s, sampc;

  handle = libeep_read_with_external_triggers(filename);
  if(handle == -1) {
    fprintf(stderr, "error opening %s", filename);
  }

  // channels
  chanc = libeep_get_channel_count(handle);
  printf("channels: %i\n", chanc);

  // samples
  sampc = 10; // libeep_get_sample_count(handle);
  for(s=0;s<sampc;++s) {
    float * sample = libeep_get_samples(handle, s, s+1);
    printf("sample[%5lu]:", s);
    for(c=0;c<chanc;++c) {
      printf(" %f", sample[c]);
    }
    free(sample);
    printf("\n");
  }

  // triggers
  triggerc = libeep_get_trigger_count(handle); 
  printf("triggers: %i\n", triggerc);
  for(i=0;i<triggerc;++i) {
    const char * code;
    uint64_t     offset;
    code = libeep_get_trigger(handle, i, & offset);
    printf("trigger(%i, %s, %lu)\n", i, code, offset);
  }

  // close
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
