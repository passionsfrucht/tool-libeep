// system
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
// libeep
#include <v4/eep.h>
#include <cnt/cnt.h>
#include <eep/eepio.h> // for the definition of eepio_fopen
#include <cnt/cnt_private.h> // for the definition of eegchan_s
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

struct _libeep_channels {
	eegchan_t *channels;
	short count;
};

static struct _libeep_entry ** _libeep_entry_map;
static struct record_info_s ** _libeep_recinfo_map;
static struct _libeep_channels ** _libeep_channel_map;

static int _libeep_entry_size;
static int _libeep_recinfo_size;
static int _libeep_channel_size;
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_entry_map and _libeep_entry_size */
static cntfile_t
_libeep_allocate() {
	struct _libeep_entry **new_entry_map = NULL;
  new_entry_map = realloc(_libeep_entry_map, sizeof(struct _libeep_entry *) * (_libeep_entry_size + 1));
  if (new_entry_map == NULL) {
	  return -1;
  }
  _libeep_entry_map = new_entry_map;
  _libeep_entry_map[_libeep_entry_size]=(struct _libeep_entry *)malloc(sizeof(struct _libeep_entry));
  if (_libeep_entry_map[_libeep_entry_size] == NULL) {
	  return -1;
  }
  _libeep_entry_map[_libeep_entry_size]->open_mode=om_none;
  _libeep_entry_map[_libeep_entry_size]->data_type=dt_none;
  _libeep_entry_size += 1;
  return _libeep_entry_size - 1;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_entry_map and _libeep_entry_size */
static void
_libeep_free(cntfile_t handle) {
  if(_libeep_entry_map[handle]==NULL) {
    fprintf(stderr, "libeep: cannot free cnt handle %i\n", handle);
    return;
  }
  // close handle
  free(_libeep_entry_map[handle]);
  // set null
  _libeep_entry_map[handle]=NULL;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_entry_map and _libeep_entry_size */
static void
_libeep_free_map() {
    int i;
	for (i = 0; i < _libeep_entry_size; ++i) {
		if (_libeep_entry_map[i] != NULL) {
			_libeep_free(i); // TODO: or use libeep_close?
		}
	}
	if (_libeep_entry_map != NULL) {
		free(_libeep_entry_map);
	}
	_libeep_entry_map = NULL;
	_libeep_entry_size = 0;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_entry_map and _libeep_entry_size */
static struct _libeep_entry *
_libeep_get_object(cntfile_t handle, open_mode om) {
	struct _libeep_entry *rv = NULL;
	if (handle < 0) {
		fprintf(stderr, "libeep: invalid cnt handle %i\n", handle);
		exit(-1);
	}
	if (_libeep_entry_map==NULL) {
		fprintf(stderr, "libeep: cnt entry map not initialized\n");
		exit(-1);
	}
	if (handle >= _libeep_entry_size) {
		fprintf(stderr, "libeep: invalid cnt handle %i\n", handle);
		exit(-1);
	}
	rv = _libeep_entry_map[handle];
  // check valid handle
  if(rv==NULL) {
    fprintf(stderr, "libeep: invalid cnt handle %i\n", handle);
    exit(-1);
  }
  // check valid open mode
  if(om != om_none && rv->open_mode != om) {
    fprintf(stderr, "libeep: invalid mode on cnt handle %i\n", handle);
    exit(-1);
  }
  return rv;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_recinfo_map and _libeep_recinfo_size */
static recinfo_t
_libeep_recinfo_allocate() {
	struct record_info_s ** new_recinfo_map = NULL;
	new_recinfo_map = realloc(_libeep_recinfo_map, sizeof(struct record_info_s *) * (_libeep_recinfo_size + 1));
	if (new_recinfo_map == NULL) {
		return -1;
	}
	_libeep_recinfo_map = new_recinfo_map;
	_libeep_recinfo_map[_libeep_recinfo_size] = (struct record_info_s *)malloc(sizeof(struct record_info_s));
	if (_libeep_recinfo_map[_libeep_recinfo_size] == NULL) {
		return -1;
	}
	memset(_libeep_recinfo_map[_libeep_recinfo_size], 0, sizeof(struct record_info_s));
	_libeep_recinfo_size += 1;
	return _libeep_recinfo_size - 1;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_recinfo_map and _libeep_recinfo_size */
static void
_libeep_recinfo_free(recinfo_t handle) {
	if (_libeep_recinfo_map[handle] == NULL) {
		fprintf(stderr, "libeep: cannot free recording info handle %i\n", handle);
		return;
	}
	// close handle
	free(_libeep_recinfo_map[handle]);
	// set null
	_libeep_recinfo_map[handle] = NULL;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_recinfo_map and _libeep_recinfo_size */
static struct record_info_s *
_libeep_get_recinfo(recinfo_t handle) {
	struct record_info_s *rv = NULL;
	if (handle < 0) {
		fprintf(stderr, "libeep: invalid recording info handle %i\n", handle);
		exit(-1);
	}
	if (_libeep_recinfo_map == NULL) {
		fprintf(stderr, "libeep: recording info map not initialized\n");
		exit(-1);
	}
	if (handle >= _libeep_recinfo_size) {
		fprintf(stderr, "libeep: invalid recording info handle %i\n", handle);
		exit(-1);
	}
	rv = _libeep_recinfo_map[handle];
	// check valid handle
	if (rv == NULL) {
		fprintf(stderr, "libeep: invalid recording info handle %i\n", handle);
		exit(-1);
	}
	return rv;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_recinfo_map and _libeep_recinfo_size */
static void
_libeep_free_recinfo_map() {
    int i;
	for (i = 0; i < _libeep_recinfo_size; ++i) {
		if (_libeep_recinfo_map[i] != NULL) {
			_libeep_recinfo_free(i);
		}
	}
	if (_libeep_recinfo_map != NULL) {
		free(_libeep_recinfo_map);
	}
	_libeep_recinfo_map = NULL;
	_libeep_recinfo_size = 0;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_channel_map and _libeep_channel_size */
static chaninfo_t
_libeep_channels_allocate() {
	struct _libeep_channels ** new_channel_map = NULL;
	new_channel_map = realloc(_libeep_channel_map, sizeof(struct _libeep_channels *) * (_libeep_channel_size + 1));
	if (new_channel_map == NULL) {
		return -1;
	}
	_libeep_channel_map = new_channel_map;
	_libeep_channel_map[_libeep_channel_size] = (struct _libeep_channels *)malloc(sizeof(struct _libeep_channels));
	if (_libeep_channel_map[_libeep_channel_size] == NULL) {
		return -1;
	}
	_libeep_channel_map[_libeep_channel_size]->channels = NULL;
	_libeep_channel_map[_libeep_channel_size]->count = 0;
	_libeep_channel_size += 1;
	return _libeep_channel_size - 1;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_channel_map and _libeep_channel_size */
static void
_libeep_channels_free(chaninfo_t handle) {
	if (_libeep_channel_map[handle] == NULL) {
		fprintf(stderr, "libeep: cannot free channel info handle %i\n", handle);
		return;
	}
	if (_libeep_channel_map[handle]->channels != NULL) {
		free(_libeep_channel_map[handle]->channels);
	}
	// close handle
	free(_libeep_channel_map[handle]);
	// set null
	_libeep_channel_map[handle] = NULL;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_channel_map and _libeep_channel_size */
static struct _libeep_channels *
_libeep_get_channels(chaninfo_t handle) {
	struct _libeep_channels *rv = NULL;
	if (handle < 0) {
		fprintf(stderr, "libeep: invalid channel info handle %i\n", handle);
		exit(-1);
	}
	if (_libeep_channel_map == NULL) {
		fprintf(stderr, "libeep: channel info map not initialized\n");
		exit(-1);
	}
	if (handle >= _libeep_channel_size) {
		fprintf(stderr, "libeep: invalid channel info handle %i\n", handle);
		exit(-1);
	}
	rv = _libeep_channel_map[handle];
	// check valid handle
	if (rv == NULL) {
		fprintf(stderr, "libeep: invalid channel info handle %i\n", handle);
		exit(-1);
	}
	return rv;
}
///////////////////////////////////////////////////////////////////////////////
/* local helper for manipulating _libeep_channel_map and _libeep_channel_size */
static void
_libeep_free_channels_map() {
    int i;
	for (i = 0; i < _libeep_channel_size; ++i) {
		if (_libeep_channel_map[i] != NULL) {
			_libeep_channels_free(i);
		}
	}
	if (_libeep_channel_map != NULL) {
		free(_libeep_channel_map);
	}
	_libeep_channel_map = NULL;
	_libeep_channel_size = 0;
}
///////////////////////////////////////////////////////////////////////////////
void libeep_init() {
	_libeep_entry_map = NULL;
	_libeep_entry_size = 0;
	_libeep_recinfo_map = NULL;
	_libeep_recinfo_size = 0;
	_libeep_channel_map = NULL;
	_libeep_channel_size = 0;
}
///////////////////////////////////////////////////////////////////////////////
void libeep_exit() {
	_libeep_free_map();
	_libeep_free_recinfo_map();
	_libeep_free_channels_map();
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_version() {
	return LIBEEP_VERSION;
}
///////////////////////////////////////////////////////////////////////////////
cntfile_t
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
cntfile_t
libeep_write_cnt(const char *filename, int rate, chaninfo_t channel_info_handle, recinfo_t recinfo_handle) {
  eegchan_t *channel_structure;
  int handle=_libeep_allocate();
  struct _libeep_entry * obj=_libeep_get_object(handle, om_none);
  struct _libeep_channels * channels_obj = _libeep_get_channels(channel_info_handle);
  // open file
  obj->file=eepio_fopen(filename, "wb");
  if(obj->file==NULL) {
    fprintf(stderr, "libeep: cannot open(1) %s\n", filename);
    return -1;
  }
  // channel setup
  channel_structure = eep_chan_init(channels_obj->count);
  if(channel_structure==NULL) {
    fprintf(stderr, "error in eep_chan_init!\n");
    return -1;
  }
  memmove(channel_structure, channels_obj->channels, sizeof(eegchan_t)* channels_obj->count);
  // file init
  obj->eep = eep_init_from_values(1.0 / (double)rate, channels_obj->count, channel_structure);
  if(obj->eep==NULL) {
    fprintf(stderr, "error in eep_init_from_values!\n");
    return -1;
  }
  if (recinfo_handle > -1) {
	  struct record_info_s * info = _libeep_get_recinfo(recinfo_handle);
	  eep_set_recording_info(obj->eep, info);
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
  eep_set_keep_file_consistent(obj->eep, 1);
  // scalings
  obj->scales = (float *)malloc(sizeof(float)* channels_obj->count);
  // housekeeping
  obj->open_mode=om_write;
  obj->data_type=dt_cnt;
  return handle;
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_close(cntfile_t handle) {
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
libeep_get_channel_count(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chanc(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_label(cntfile_t handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_label(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_unit(cntfile_t handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_unit(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_channel_reference(cntfile_t handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_reflab(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
float
libeep_get_channel_scale(cntfile_t handle, int index) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return (float)eep_get_chan_scale(obj->eep, index);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_channel_index(cntfile_t handle, const char *chan) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_chan_index(obj->eep, chan);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_sample_frequency(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return (int)(/* TODO: round before truncating */(1.0 / eep_get_period(obj->eep)));
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_sample_count(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  return eep_get_samplec(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
static float *
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
  buffer_unscaled = (float *)malloc(FLOAT_CNTBUF_SIZE(obj->eep, to-from));
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
static float *
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
libeep_get_samples(cntfile_t handle, long from, long to) {
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
libeep_add_samples(cntfile_t handle, const float *data, int n) {
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
libeep_add_raw_samples(cntfile_t handle, const int32_t *data, int n) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    eep_write_sraw(obj->eep, data, n);
}
///////////////////////////////////////////////////////////////////////////////
int32_t *
libeep_get_raw_samples(cntfile_t handle, long from, long to) {
    sraw_t *buffer_unscaled;
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
recinfo_t
libeep_create_recinfo()
{
	return _libeep_recinfo_allocate();
}
///////////////////////////////////////////////////////////////////////////////
time_t
libeep_get_start_time(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_recording_startdate_epoch(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void 
libeep_get_start_date_and_fraction(recinfo_t handle, double* start_date, double* start_fraction) {
	if (start_date) *start_date = 0.0;
	if (start_fraction) *start_fraction = 0.0;
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
	record_info_t rec_inf;
	if (eep_has_recording_info(obj->eep)) {
		eep_get_recording_info(obj->eep, &rec_inf);
		if (start_date) *start_date = rec_inf.m_startDate;
		if (start_fraction) *start_fraction = rec_inf.m_startFraction;
	}
}

///////////////////////////////////////////////////////////////////////////////
void
libeep_set_start_time(recinfo_t handle, time_t start_time) {
	struct record_info_s * obj = _libeep_get_recinfo(handle);
	eep_unixdate_to_exceldate(start_time, &obj->m_startDate, &obj->m_startFraction);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_start_date_and_fraction(recinfo_t handle, double start_date, double start_fraction) {
	struct record_info_s * obj = _libeep_get_recinfo(handle);
	obj->m_startDate = start_date;
	obj->m_startFraction = start_fraction;
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_hospital(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_hospital(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_hospital(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szHospital) / sizeof(obj->m_szHospital[0]) - 1;
		strncpy(obj->m_szHospital, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_test_name(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_test_name(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_test_name(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szTestName) / sizeof(obj->m_szTestName[0]) - 1;
		strncpy(obj->m_szTestName, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_test_serial(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_test_serial(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_test_serial(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szTestSerial) / sizeof(obj->m_szTestSerial[0]) - 1;
		strncpy(obj->m_szTestSerial, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_physician(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_physician(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_physician(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szPhysician) / sizeof(obj->m_szPhysician[0]) - 1;
		strncpy(obj->m_szPhysician, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_technician(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_technician(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_technician(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szTechnician) / sizeof(obj->m_szTechnician[0]) - 1;
		strncpy(obj->m_szTechnician, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_machine_make(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_machine_make(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_machine_make(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szMachineMake) / sizeof(obj->m_szMachineMake[0]) - 1;
		strncpy(obj->m_szMachineMake, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_machine_model(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_machine_model(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_machine_model(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szMachineModel) / sizeof(obj->m_szMachineModel[0]) - 1;
		strncpy(obj->m_szMachineModel, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_machine_serial_number(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_machine_serial_number(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_machine_serial_number(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szMachineSN) / sizeof(obj->m_szMachineSN[0]) - 1;
		strncpy(obj->m_szMachineSN, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_name(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_name(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_name(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szName) / sizeof(obj->m_szName[0]) - 1;
		strncpy(obj->m_szName, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_id(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_id(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_id(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szID) / sizeof(obj->m_szID[0]) - 1;
		strncpy(obj->m_szID, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_address(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_address(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_address(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szAddress) / sizeof(obj->m_szAddress[0]) - 1;
		strncpy(obj->m_szAddress, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_patient_phone(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_phone(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_phone(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szPhone) / sizeof(obj->m_szPhone[0]) - 1;
		strncpy(obj->m_szPhone, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_comment(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_comment(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_comment(recinfo_t handle, const char *value) {
	if (value) {
		struct record_info_s * obj = _libeep_get_recinfo(handle);
		const size_t len = sizeof(obj->m_szComment) / sizeof(obj->m_szComment[0]) - 1;
		strncpy(obj->m_szComment, value, len);
	}
}
///////////////////////////////////////////////////////////////////////////////
char
libeep_get_patient_sex(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_sex(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_sex(recinfo_t handle, char value) {
	struct record_info_s * obj = _libeep_get_recinfo(handle);
	obj->m_chSex = value;
}
///////////////////////////////////////////////////////////////////////////////
char
libeep_get_patient_handedness(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return eep_get_patient_handedness(obj->eep);
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_patient_handedness(recinfo_t handle, char value) {
	struct record_info_s * obj = _libeep_get_recinfo(handle);
	obj->m_chHandedness = value;
}
///////////////////////////////////////////////////////////////////////////////
time_t
libeep_get_date_of_birth(cntfile_t handle) {
	struct tm *dob = NULL;
	struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
	dob = eep_get_patient_day_of_birth(obj->eep);
	return mktime(dob); // performs the reverse translation that localtime does so libeep_set_date_of_birth have to use localtime to get the correct date back
}
///////////////////////////////////////////////////////////////////////////////
void
libeep_set_date_of_birth(recinfo_t handle, time_t value) {
	struct record_info_s * obj = _libeep_get_recinfo(handle);
	struct tm *temp = localtime(&value);
	memmove(&obj->m_DOB, temp, sizeof(struct tm));
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_add_trigger(cntfile_t handle, uint64_t sample, const char *code) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_write);
    return trg_set(eep_get_trg(obj->eep), sample, code);
}
///////////////////////////////////////////////////////////////////////////////
int
libeep_get_trigger_count(cntfile_t handle) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return trg_get_c(eep_get_trg(obj->eep));
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_trigger(cntfile_t handle, int idx, uint64_t *sample) {
    struct _libeep_entry * obj = _libeep_get_object(handle, om_read);
    return trg_get(eep_get_trg(obj->eep), idx, sample);
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_zero_offset(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return (int)(libeep_get_sample_frequency(handle) * eep_get_pre_stimulus_interval(obj->eep));
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_condition_label(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_conditionlabel(obj->eep);
  }
  return "none";
}
///////////////////////////////////////////////////////////////////////////////
const char *
libeep_get_condition_color(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_conditioncolor(obj->eep);
  }
  return "none";
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_trials_total(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_total_trials(obj->eep);
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
long
libeep_get_trials_averaged(cntfile_t handle) {
  struct _libeep_entry * obj=_libeep_get_object(handle, om_read);
  if(obj->data_type==dt_avr) {
    return eep_get_averaged_trials(obj->eep);
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////
chaninfo_t libeep_create_channel_info() {
	return _libeep_channels_allocate();
}
///////////////////////////////////////////////////////////////////////////////
int libeep_add_channel(chaninfo_t handle, const char *label, const char *ref_label, const char *unit) {
	eegchan_t *channels = NULL;
	const char *default_ref_label = "ref";
	const char *default_unit = "uV";
	struct _libeep_channels * obj = _libeep_get_channels(handle);
	// the channel label shall have a value; ref_label and unit might be NULL
	if (label == NULL) {
		return obj->count;
	}
	if (ref_label == NULL) {
		ref_label = default_ref_label;
	}
	if (unit == NULL) {
		unit = default_unit;
	}
	channels = (eegchan_t *)realloc(obj->channels, sizeof(eegchan_t) * (obj->count + 1));
	if (channels == NULL) {
		return obj->count;
	}
	obj->channels = channels;
	eep_chan_set(obj->channels, obj->count, label, 1, 1.0 / SCALING_FACTOR, unit);
	eep_chan_set_reflab(obj->channels, obj->count, ref_label);
	obj->count += 1;
	return obj->count;
}
