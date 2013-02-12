// system
#include <stdlib.h>
#include <stdio.h>
// libeep
#include <v4/eep.h>
#include <eep/version.h>
#include <cnt/cnt.h>
///////////////////////////////////////////////////////////////////////////////
struct _libeep_entry {
  FILE  * file;
  eeg_t * eep;
  float * scales;
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
_libeep_get_object(int handle) {
  struct _libeep_entry *rv=_libeep_entry_map[handle];
  if(rv==NULL || handle >= _libeep_entry_size) {
    fprintf(stderr, "libeep: invalid handle %i\n", handle);
    exit(-1);
  }
  return rv;
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_read(const char *filename) {
  int status;
  int handle=_libeep_allocate();
  struct _libeep_entry * obj=_libeep_get_object(handle);
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
  int channel_id,
      channel_count=eep_get_chanc(obj->eep);
  obj->scales=(float *)malloc(sizeof(float) * channel_count);
  for(channel_id=0;channel_id<channel_count;channel_id++) {
    obj->scales[channel_id]=(float)eep_get_chan_scale(obj->eep, channel_id);
  }
  // read triggers(if CNT)
  // read rejections(if CNT)
  return handle;
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_close(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  // close scales
  free(_libeep_entry_map[handle]->scales);
  eep_free(obj->eep);
  fclose(obj->file);
  _libeep_free(handle);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_channel_count(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  return eep_get_chanc(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_label(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  return eep_get_chan_label(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_unit(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  return eep_get_chan_unit(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_reference(int handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  return eep_get_chan_reflab(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
float
libeep_get_sample_frequency(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  return 1.0 / eep_get_period(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_sample_count(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  return eep_get_samplec(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
float *
_libeep_get_samples_avr(struct _libeep_entry * obj, long from, long to) {
  // seek
  if(eep_seek(obj->eep, DATATYPE_AVERAGE, from, 0)) {
    return NULL;
  }
  // get unscaled data
  float *buffer_unscaled;
  buffer_unscaled = (float *)malloc(CNTBUF_SIZE(obj->eep, to-from));
  if(eep_read_float(obj->eep, DATATYPE_AVERAGE, buffer_unscaled, to-from)) {
    free(buffer_unscaled);
    return NULL;
  }
  // scale data
  float * buffer_scaled = (float *)malloc(sizeof(float) * (to-from) * eep_get_chanc(obj->eep));
  const float * ptr_src=buffer_unscaled,
              * ptr_scales=obj->scales;
        float * ptr_dst=buffer_scaled;
  int n=eep_get_chanc(obj->eep) * (to-from),
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
  // seek
  if(eep_seek(obj->eep, DATATYPE_EEG, from, 0)) {
    return NULL;
  }
  // get unscaled data
  sraw_t *buffer_unscaled;
  buffer_unscaled = (sraw_t *)malloc(CNTBUF_SIZE(obj->eep, to-from));
  if(eep_read_sraw(obj->eep, DATATYPE_EEG, buffer_unscaled, to-from)) {
    free(buffer_unscaled);
    return NULL;
  }
  // scale data
  float * buffer_scaled = (float *)malloc(sizeof(float) * (to-from) * eep_get_chanc(obj->eep));
  const sraw_t * ptr_src=buffer_unscaled;
  const float  * ptr_scales=obj->scales;
        float  * ptr_dst=buffer_scaled;
  int n=eep_get_chanc(obj->eep) * (to-from),
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
  struct _libeep_entry * obj=_libeep_get_object(handle);
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) { return _libeep_get_samples_avr(obj ,from, to); }
  if(eep_has_data_of_type(obj->eep, DATATYPE_EEG))     { return _libeep_get_samples_cnt(obj ,from, to); }
  return NULL;
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_zero_offset(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) {
    return (int)(libeep_get_sample_frequency(handle) * eep_get_pre_stimulus_interval(obj->eep));
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_condition_label(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) {
    return eep_get_conditionlabel(obj->eep);
  }
  return "none";
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_condition_color(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) {
    return eep_get_conditioncolor(obj->eep);
  }
  return "none";
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_trials_total(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) {
    return eep_get_total_trials(obj->eep);
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_trials_averaged(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle);
  if(eep_has_data_of_type(obj->eep, DATATYPE_AVERAGE)) {
    return eep_get_averaged_trials(obj->eep);
  }
  return 0;
}
