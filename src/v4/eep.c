// system
#include <stdlib.h>
#include <stdio.h>
// libeep
#include <v4/eep.h>
#include <cnt/cnt.h>
///////////////////////////////////////////////////////////////////////////////
#define SCALING_FACTOR 128
///////////////////////////////////////////////////////////////////////////////
typedef enum { dt_none, dt_avr, dt_cnt } data_type;
typedef enum { om_none, om_read, om_write } open_mode;
///////////////////////////////////////////////////////////////////////////////
struct _libeep_entry {
  FILE      * file;
  eeg_t     * eep;
  data_type   data_type;
  open_mode   open_mode;
  float     * scales;
};

struct _libeep_entry ** _libeep_entry_map;

int _libeep_entry_size;
///////////////////////////////////////////////////////////////////////////////
void libeep_init() {
  _libeep_entry_map=NULL;
  _libeep_entry_size=0;
}
///////////////////////////////////////////////////////////////////////////////
void libeep_exit() {
}
///////////////////////////////////////////////////////////////////////////////
int
_libeep_allocate() {
  _libeep_entry_size+=1;
  _libeep_entry_map=realloc(_libeep_entry_map, sizeof(struct _libeep_entry *) * _libeep_entry_size);
  _libeep_entry_map[_libeep_entry_size-1]=(struct _libeep_entry *)malloc(sizeof(struct _libeep_entry));
  _libeep_entry_map[_libeep_entry_size-1]->open_mode=om_none;
  _libeep_entry_map[_libeep_entry_size-1]->data_type=dt_none;
  return _libeep_entry_size-1;
}
///////////////////////////////////////////////////////////////////////////////
void
_libeep_free(int handle) {
  if(_libeep_entry_map[handle]==NULL) {
    fprintf(stderr, "libeep: cannot free handle %i\n", handle);
    return;
  }
  // close handle
  free(_libeep_entry_map[handle]);
  // set null
  _libeep_entry_map[handle]=NULL;
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_version() {
  return LIBEEP_VERSION;
}
///////////////////////////////////////////////////////////////////////////////
struct _libeep_entry *
_libeep_get_object(int handle, open_mode om) {
  struct _libeep_entry *rv=_libeep_entry_map[handle];
  // check valid handle
  if(rv==NULL || handle >= _libeep_entry_size) {
    fprintf(stderr, "libeep: invalid handle %i\n", handle);
    exit(-1);
  }
  // check valid open mode 
  if(om != om_none && rv->open_mode != om) {
    fprintf(stderr, "libeep: invalid mode on handle %i\n", handle);
    exit(-1);
  }
  return rv;
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_read(const char *filename) {
  int status;
  int handle=_libeep_allocate();
  int channel_id;
  int channel_count;
  struct _libeep_entry * obj=_libeep_get_object(handle, om_none);
  // open file
  obj->file=fopen(filename, "rb");
  if(obj->file==NULL) {
    fprintf(stderr, "libeep: cannot open(1) %s\n", filename);
    return -1;
  }
  // eep struct
  obj->eep=eep_init_from_file(filename, obj->file, &status);
  if(status != CNTERR_NONE) {
    fprintf(stderr, "libeep: cannot open(2) %s\n", filename);
    return -1;
  }
  // read channel scale
  channel_count=eep_get_chanc(obj->eep);
  obj->scales=(float *)malloc(sizeof(float) * channel_count);
  for(channel_id=0;channel_id<channel_count;channel_id++) {
    obj->scales[channel_id]=(float)eep_get_chan_scale(obj->eep, channel_id);
  }
  // read triggers(if CNT)
  // read rejections(if CNT)
  // housekeeping
  obj->open_mode=om_read;
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) { obj->data_type=dt_avr; }
  if(eep_has_data_of_type(obj->eep, DATATYPE_EEG))     { obj->data_type=dt_cnt; }
  return handle;
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_write_cnt(const char *filename, int rate, int nchan) {
  eegchan_t *channel_structure;
  int handle=_libeep_allocate();
  int channel_id;
  struct _libeep_entry * obj=_libeep_get_object(handle, om_none);
  // open file
  obj->file=fopen(filename, "wb");
  if(obj->file==NULL) {
    fprintf(stderr, "libeep: cannot open(1) %s\n", filename);
    return -1;
  }
  // channel setup
  channel_structure=eep_chan_init(nchan);
  if(channel_structure==NULL) {
    fprintf(stderr, "error in eep_chan_init!\n");
    return -1;
  }
  for(channel_id=0;channel_id<nchan;channel_id++) {
    char s[8];
    sprintf(s, "chan%04i", channel_id);
    eep_chan_set(channel_structure, channel_id, s, 1, 1.0/SCALING_FACTOR, "uV");
  }
  // file init
  obj->eep=eep_init_from_values(1/(float)rate, nchan, channel_structure);
  if(obj->eep==NULL) {
    fprintf(stderr, "error in eep_init_from_values!\n");
    return -1;
  }
  // eep struct
  if(eep_create_file(obj->eep, filename, obj->file, NULL, 0, filename) != CNTERR_NONE) {
    fprintf(stderr, "could not create file!\n");
    return -1;
  }
  // switch writing mode
  if(eep_prepare_to_write(obj->eep, DATATYPE_EEG, rate, NULL) != CNTERR_NONE) {
    fprintf(stderr, "could prepare file!\n");
    return -1;
  }
  // scalings
  obj->scales=(float *)malloc(sizeof(float) * nchan);
  // housekeeping
  obj->open_mode=om_write;
  obj->data_type=dt_cnt;
  return handle;
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_close(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_none);
  // close writing
  if(obj->open_mode==om_write) {
    eep_finish_file(obj->eep);
  }
  // close reading
  if(obj->open_mode==om_read) {
    eep_free(obj->eep);
  }
  // close scales
  free(_libeep_entry_map[handle]->scales);
  // cleanup
  fclose(obj->file);
  _libeep_free(handle);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_channel_count(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chanc(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_label(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_label(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_unit(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_unit(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_reference(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_reflab(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
float
libeep_get_channel_scale(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return (float)eep_get_chan_scale(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_channel_index(int handle, const char *chan) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_index(obj->eep, chan);
}
///////////////////////////////////////////////////////////////////////////////
float
libeep_get_sample_frequency(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return (float)(1.0 / eep_get_period(obj->eep));
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_sample_count(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_samplec(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
float *
_libeep_get_samples_avr(struct _libeep_entry * obj, long from, long to) {
  float *buffer_unscaled,
        *buffer_scaled;
  const float * ptr_src,
              * ptr_scales;
        float * ptr_dst;
  int n;
  int w;
  // seek
  if(eep_seek(obj->eep, DATATYPE_AVERAGE, from, 0)) {
    return NULL;
  }
  // get unscaled data
  buffer_unscaled = (float *)malloc(CNTBUF_SIZE(obj->eep, to-from));
  if(eep_read_float(obj->eep, DATATYPE_AVERAGE, buffer_unscaled, to-from)) {
    free(buffer_unscaled);
    return NULL;
  }
  // scale data
  buffer_scaled = (float *)malloc(sizeof(float) * (to-from) * eep_get_chanc(obj->eep));
  ptr_src=buffer_unscaled,
  ptr_scales=obj->scales;
  ptr_dst=buffer_scaled;
  n=eep_get_chanc(obj->eep) * (to-from);
  w = 0;
  while(n--) {
    if(!w) {
      w=to-from;
      ptr_scales=obj->scales;
    }
    *ptr_dst++ = (float)(*ptr_src++) * *ptr_scales++;
    w--;
  }
  free(buffer_unscaled);
  return buffer_scaled;
  // TODO
}
///////////////////////////////////////////////////////////////////////////////
float *
_libeep_get_samples_cnt(struct _libeep_entry * obj, long from, long to) {
  sraw_t *buffer_unscaled;
  float * buffer_scaled;
  const sraw_t * ptr_src;
  const float  * ptr_scales;
        float  * ptr_dst;
  int n;
  int w;
  // seek
  if(eep_seek(obj->eep, DATATYPE_EEG, from, 0)) {
    return NULL;
  }
  // get unscaled data
  buffer_unscaled = (sraw_t *)malloc(CNTBUF_SIZE(obj->eep, to-from));
  if(eep_read_sraw(obj->eep, DATATYPE_EEG, buffer_unscaled, to-from)) {
    free(buffer_unscaled);
    return NULL;
  }
  // scale data
  buffer_scaled = (float *)malloc(sizeof(float) * (to-from) * eep_get_chanc(obj->eep));
  ptr_src=buffer_unscaled;
  ptr_scales=obj->scales;
  ptr_dst=buffer_scaled;
  n=eep_get_chanc(obj->eep) * (to-from);
  w = 0;
  while(n--) {
    if(!w) {
      w=to-from;
      ptr_scales=obj->scales;
    }
    *ptr_dst++ = (float)(*ptr_src++) * *ptr_scales++;
    w--;
  }
  free(buffer_unscaled);
  return buffer_scaled;
}
///////////////////////////////////////////////////////////////////////////////
float *
libeep_get_samples(int handle, long from, long to) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) { return _libeep_get_samples_avr(obj, from, to); }
  if(obj->data_type==dt_cnt) { return _libeep_get_samples_cnt(obj, from, to); }
  return NULL;
}
///////////////////////////////////////////////////////////////////////////////
void libeep_add_samples(int handle, const float *data, int n) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_write);
  sraw_t *buffer;
  const float  * ptr_src;
        sraw_t * ptr_dst;
  int c;

  c=CNTBUF_SIZE(obj->eep, n);
  buffer=(sraw_t*)malloc(c);
  ptr_src=data;
  ptr_dst=buffer;

  c/=sizeof(sraw_t);
  while(c--) {
    *ptr_dst++ = (sraw_t)(*ptr_src++ * SCALING_FACTOR);
  }

  eep_write_sraw(obj->eep, buffer, n);

  free(buffer);
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_zero_offset(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return (int)(libeep_get_sample_frequency(handle) * eep_get_pre_stimulus_interval(obj->eep));
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_condition_label(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_conditionlabel(obj->eep);
  }
  return "none";
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_condition_color(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_conditioncolor(obj->eep);
  }
  return "none";
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_trials_total(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_total_trials(obj->eep);
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_trials_averaged(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_averaged_trials(obj->eep);
  }
  return 0;
}
