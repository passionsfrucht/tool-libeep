// system
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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
  obj->file=eepio_fopen(filename, "rb");
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
/* local helper functions */
static void
channel_label(int channel, const char **channel_labels, char *result) {
    if (!channel_labels) {
        sprintf(result, "chan%04i", channel);
    }
    else {
        strncpy(result, channel_labels[channel], 10);
        result[9] = '\0';
    }
}
///////////////////////////////////////////////////////////////////////////////
static void
channel_unit(int channel, const char **channel_units, char *result) {
    if (!channel_units) {
        sprintf(result, "uV");
    }
    else {
        strncpy(result, channel_units[channel], 10);
        result[9] = '\0';
    }
}
///////////////////////////////////////////////////////////////////////////////
static void
libeep_create_recording_info(eeg_t *cnt) {
    record_info_t *info = (record_info_t *)malloc(sizeof(record_info_t));
    if (info) {
        memset(info, 0, sizeof(record_info_t));
        eep_set_recording_info(cnt, info);
    }
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_write_cnt(const char *filename, int rate, int nchan, const char **channel_labels, const char **channel_units) {
  eegchan_t *channel_structure;
  int handle=_libeep_allocate();
  int channel_id;
  struct _libeep_entry * obj=_libeep_get_object(handle, om_none);
  // open file
  obj->file=eepio_fopen(filename, "wb");
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
    char label[10];
    char unit[10];
    channel_label(channel_id, channel_labels, label);
    channel_unit(channel_id, channel_units, unit);
    eep_chan_set(channel_structure, channel_id, label, 1, 1.0 / SCALING_FACTOR, unit);
  }
  // file init
  obj->eep=eep_init_from_values(1/(float)rate, nchan, channel_structure);
  if(obj->eep==NULL) {
    fprintf(stderr, "error in eep_init_from_values!\n");
    return -1;
  }
  // eep struct
  if(eep_create_file64(obj->eep, filename, obj->file, filename) != CNTERR_NONE) {
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
  eepio_fclose(obj->file);
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
int
libeep_get_sample_frequency(int handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return (int)(/* TODO: round before truncating */(1.0 / eep_get_period(obj->eep)));
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
void
libeep_free_samples(float *buffer) {
  if(buffer) {
    free(buffer);
  }
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_add_samples(int handle, const float *data, int n) {
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
void
libeep_add_raw_samples(int handle, const int32_t *data, int n) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    eep_write_sraw(obj->eep, data, n);
}
///////////////////////////////////////////////////////////////////////////////
int32_t *
libeep_get_raw_samples(int handle, long from, long to) {
    sraw_t *buffer_unscaled;
    const sraw_t * ptr_src;
    struct _libeep_entry * obj;

    obj = _libeep_get_object(handle, om_read);
    // seek
    if (eep_seek(obj->eep, DATATYPE_EEG, from, 0)) {
        return NULL;
    }
    // get unscaled data
    buffer_unscaled = (sraw_t *)malloc(CNTBUF_SIZE(obj->eep, to - from));
    if (eep_read_sraw(obj->eep, DATATYPE_EEG, buffer_unscaled, to - from)) {
        free(buffer_unscaled);
        return NULL;
    }
    return buffer_unscaled;
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_free_raw_samples(int32_t *buffer) {
  if(buffer) {
    free(buffer);
  }
}
///////////////////////////////////////////////////////////////////////////////
time_t
libeep_get_start_time(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_recording_startdate_epoch(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_start_time(int handle, time_t start_time) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_recording_startdate_epoch(obj->eep, start_time);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_hospital(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_hospital(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_hospital(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_hospital(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_test_name(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_test_name(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_test_name(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_test_name(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_test_serial(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_test_serial(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_test_serial(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_test_serial(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_physician(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_physician(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_physician(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_physician(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_technician(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_technician(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_technician(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_technician(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_machine_make(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_machine_make(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_machine_make(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_machine_make(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_machine_model(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_machine_model(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_machine_model(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_machine_model(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_machine_serial_number(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_machine_serial_number(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_machine_serial_number(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_machine_serial_number(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_name(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_name(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_name(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_patient_name(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_id(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_id(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_id(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_patient_id(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_address(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_address(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_address(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_patient_address(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_phone(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_phone(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_phone(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_patient_phone(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_comment(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_comment(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_comment(int handle, const char *value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_comment(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
char
libeep_get_patient_sex(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_sex(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_sex(int handle, char value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_patient_sex(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
char
libeep_get_patient_handedness(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_handedness(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_handedness(int handle, char value) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    if (!eep_has_recording_info(obj->eep)) {
        libeep_create_recording_info(obj->eep);
    }
    eep_set_patient_handedness(obj->eep, value);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_add_trigger(int handle, uint64_t sample, const char *code) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    return trg_set(eep_get_trg(obj->eep), sample, code);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_trigger_count(int handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return trg_get_c(eep_get_trg(obj->eep));
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_trigger(int handle, int idx, uint64_t *sample) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return trg_get(eep_get_trg(obj->eep), idx, sample);
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
