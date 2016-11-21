#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <cnt/evt.h>
/******************************************************************************
 * misc
 *****************************************************************************/
#define TODO_MARKER { fprintf(stderr, "TODO: %s(%i): %s\n", __FILE__, __LINE__, __FUNCTION__); exit(-1); }
/*****************************************************************************/
enum _libeep_evt_log_level { evt_log_dbg, evt_log_inf, evt_log_err };
static
void
_libeep_evt_log(int level, const char * fmt, ...) {
  if(level >= evt_log_inf) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end (args);
  }
}
/*****************************************************************************/
static
void
_libeep_evt_string_delete(char * s) {
  if(s != NULL) {
    free(s);
  }
}
/*****************************************************************************/
static
void
_libeep_evt_wstring_delete(wchar_t * s) {
  if(s != NULL) {
    free(s);
  }
}
/******************************************************************************
 * libeep evt class
 *****************************************************************************/
libeep_evt_class_t *
libeep_evt_class_new() {
  libeep_evt_class_t * rv = NULL;

  rv = (libeep_evt_class_t *)malloc(sizeof(libeep_evt_class_t));
  memset(rv, 0, sizeof(libeep_evt_class_t));

  return rv;
}
/*****************************************************************************/
void
libeep_evt_class_delete(libeep_evt_class_t * c) {
  if(c != NULL) {
    _libeep_evt_string_delete(c->name);
    free(c);
  }
}
/******************************************************************************
 * libeep evt
 *****************************************************************************/
libeep_evt_t * libeep_evt_new() {
  libeep_evt_t * rv = NULL;

  rv=(libeep_evt_t *)malloc(sizeof(libeep_evt_t));
  memset(rv, 0, sizeof(libeep_evt_t));

  return rv;
}
/*****************************************************************************/
void libeep_evt_delete(libeep_evt_t * e) {
  if(e != NULL) {
    while(e->evt_list_first) {
      libeep_evt_event_t * tmp = e->evt_list_first;
      e->evt_list_first = e->evt_list_first->next_event;
      libeep_evt_event_delete(tmp);
    }
    free(e);
  }
}
#if 0
/*****************************************************************************/
static
void
_liveep_evt_read_1byte(FILE *f, void * d) {
  if(fread(&d, 1, 1, f) != 1) {
    _libeep_evt_log(evt_log_err, "could not read 1 byte from file");
  }
}
/*****************************************************************************/
static
void
_liveep_evt_read_2byte(FILE *f, void * d) {
  if(fread(&d, 2, 1, f) != 1) {
    _libeep_evt_log(evt_log_err, "could not read 2 bytes from file");
  }
}
/*****************************************************************************/
static
void
_liveep_evt_read_4byte(FILE *f, void * d) {
  if(fread(&d, 4, 1, f) != 1) {
    _libeep_evt_log(evt_log_err, "could not read 4 bytes from file");
  }
}
/*****************************************************************************/
static
void
_liveep_evt_read_8byte(FILE *f, void * d) {
  if(fread(&d, 8, 1, f) != 1) {
    _libeep_evt_log(evt_log_err, "could not read 8 bytes from file");
  }
}
#endif
/******************************************************************************
 * internal function; add to list
 *****************************************************************************/
static
void
_libeep_evt_event_list_add_back(libeep_evt_t * l, libeep_evt_event_t *n) {
  if(l->evt_list_first == NULL) {
    l->evt_list_first = n;
    l->evt_list_last = n;
  } else {
    l->evt_list_last->next_event = n;
    l->evt_list_last             = n;
  }
}
/******************************************************************************
 * internal function; read string
 *****************************************************************************/
static
char *
_libeep_evt_read_string(FILE * f) {
  char    * rv = NULL;
  size_t    length = 0;
  uint8_t   byte = 0;
  uint16_t  word = 0;
  uint32_t  dword = 0;
  uint64_t  qword = 0;

  /* is the length encoded in a byte? */
  if(fread(&byte, sizeof(uint8_t), 1, f) == 1) {
    length = (size_t)byte;
    if(byte == 0xFF) {
      /* is the size encoded in a word? */
      if(fread(&word, sizeof(uint16_t), 1, f) == 1) {
        length = (size_t)word;
        if(word == 0xFFFF) {
          /* is the size encoded in a dword? */
          if(fread(&dword, sizeof(uint32_t), 1, f) == 1) {
            length = (size_t)dword;
            if(dword == 0xFFFFFFFF) {
              /* is the size encoded in a qword? */
              if(fread(&qword, sizeof(uint64_t), 1, f) == 1) {
                length = (size_t)qword;
              }
            }
          }
        }
      }
    }
  }

  if(length == 0) {
    return NULL;
  }

  _libeep_evt_log(evt_log_dbg, "%s: length: %li\n", __FUNCTION__, length);

  rv=(char *)malloc(length + 1);
  if(fread(rv, length, 1, f) == 1) {
    rv[length] = 0;
    _libeep_evt_log(evt_log_dbg, "%s: string: %s\n", __FUNCTION__, rv);
  } else {
    /* something went wrong reading the string */
    TODO_MARKER;
  }

  return rv;
}
/******************************************************************************
 * internal function; read wstring
 *****************************************************************************/
static
wchar_t *
_libeep_evt_read_wstring(FILE * f) {
  wchar_t * rv = NULL;
  int32_t   i = 0;
  int32_t   length = 0;
  if(fread(&length, sizeof(int32_t), 1, f) == 1) {
    rv = (wchar_t *)malloc(sizeof(wchar_t)*(length + 1));
    for(i=0;i<length;++i) {
      int16_t tmp;
      fread(&tmp, sizeof(int16_t), 1, f);
      rv[i] = (wchar_t)tmp;
    }
  }

  return rv;
}
/******************************************************************************
 * internal function; read evt class
 *****************************************************************************/
static
libeep_evt_class_t *
_libeep_evt_read_class(FILE * f) {
  libeep_evt_class_t * rv = libeep_evt_class_new();

  if(fread(&rv->tag, sizeof(int32_t), 1, f) == 1) {
    switch(rv->tag) {
      case 0:
        break;
      case -1: /* string */
        rv->name = _libeep_evt_read_string(f);
        break;
      default: /* unicode */
        break;
    }
  }

  _libeep_evt_log(evt_log_dbg, "%s: class(%i, %s)\n", __FUNCTION__, rv->tag, rv->name);

  return rv;
}
/******************************************************************************
 * libeep_evt event base
 *****************************************************************************/
libeep_evt_GUID_t *
libeep_evt_GUID_new() {
  libeep_evt_GUID_t * rv = NULL;

  rv = (libeep_evt_GUID_t *)malloc(sizeof(libeep_evt_GUID_t));
  memset(rv, 0, sizeof(libeep_evt_GUID_t));

  return rv;
}
/*****************************************************************************/
void
libeep_evt_GUID_delete(libeep_evt_GUID_t * g) {
  if(g != NULL) {
    free(g);
  }
}
/******************************************************************************
 * libeep_evt event base
 *****************************************************************************/
libeep_evt_event_t *
libeep_evt_event_new() {
  libeep_evt_event_t * rv = NULL;

  rv = (libeep_evt_event_t *)malloc(sizeof(libeep_evt_event_t));
  memset(rv, 0, sizeof(libeep_evt_event_t));

  return rv;
}
/*****************************************************************************/
void
libeep_evt_event_delete(libeep_evt_event_t * e) {
  if(e != NULL) {
    libeep_evt_GUID_delete(e->guid);
    _libeep_evt_string_delete(e->name);
    _libeep_evt_string_delete(e->user_visible_name);
    free(e);
  }
}
/******************************************************************************
 * internal function; read evt events
 *****************************************************************************/
enum vt_e {
  vt_empty = 0,
  vt_null = 1,
  vt_i2 = 2,
  vt_i4 = 3,
  vt_r4 = 4,
  vt_r8 = 5,
  vt_bstr = 8,
  vt_bool = 11,
  vt_array = 0x2000,
  vt_byref = 0x4000,
};
/*****************************************************************************/
static
int32_t
_libeep_evt_read_variant_base(FILE *f) {
  int32_t type = 0;
  if(fread(&type, sizeof(int16_t), 1, f) == 1) {
    _libeep_evt_log(evt_log_dbg, "%s: type: %i\n", __FUNCTION__, type);

    if (type == vt_empty || type == vt_null) {
    } else if (type == vt_i2) {
      int16_t tmp;
      fread(&tmp, sizeof(int16_t), 1, f);
      _libeep_evt_log(evt_log_dbg, "%s: i2: %i\n", __FUNCTION__, tmp);
    } else if (type == vt_i4) {
      int32_t tmp;
      fread(&tmp, sizeof(int32_t), 1, f);
      _libeep_evt_log(evt_log_dbg, "%s: i4: %i\n", __FUNCTION__, tmp);
    } else if (type == vt_r4) {
      float tmp;
      fread(&tmp, sizeof(float), 1, f);
      _libeep_evt_log(evt_log_dbg, "%s: r4: %i\n", __FUNCTION__, tmp);
    } else if (type == vt_r8) {
      double tmp;
      fread(&tmp, sizeof(double), 1, f);
      _libeep_evt_log(evt_log_dbg, "%s: r8: %g\n", __FUNCTION__, tmp);
    } else if (type == vt_bstr) {
      wchar_t * tmp;
      tmp = _libeep_evt_read_wstring(f);

      _libeep_evt_log(evt_log_dbg, "%s: wstring: %ls\n", __FUNCTION__, tmp);

      _libeep_evt_wstring_delete(tmp);
    } else if (type == vt_bool) {
      TODO_MARKER;
    } else {
/*
      _libeep_evt_log(evt_log_err, "%s: type: %i\n", __FUNCTION__, type);
      TODO_MARKER;
*/
    }
  }
  return type;
}
/*****************************************************************************/
static
void
_libeep_evt_read_variant_array(FILE *f) {
  int32_t variant_type = 0;

  variant_type = _libeep_evt_read_variant_base(f);

  _libeep_evt_log(evt_log_dbg, "%s: variant_type: %i\n", __FUNCTION__, variant_type);

  if (variant_type == vt_i2) {
    TODO_MARKER;
  } else if (variant_type == vt_i4) {
    TODO_MARKER;
  } else if (variant_type == vt_r4) {
    uint32_t size;
    uint32_t i;
    float    tmp;
    fread(&size, sizeof(uint32_t), 1, f);
    for(i=0;i<size;++i) {
      fread(&tmp, sizeof(float), 1, f);
    }
  } else if (variant_type == vt_r8) {
    TODO_MARKER;
  } else if (variant_type == vt_bool) {
    TODO_MARKER;
  } else if (variant_type == vt_bstr) {
    TODO_MARKER;
  }

}
/*****************************************************************************/
static
void
_libeep_evt_read_variant(FILE *f) {
  int32_t variant_type = 0;

  variant_type = _libeep_evt_read_variant_base(f);

  switch(variant_type) {
    case vt_i2:
    case vt_i4:
    case vt_r4:
    case vt_r8:
    case vt_bool:
    case vt_bstr:
      break;
    default:
      if( (variant_type & vt_byref) || (variant_type & vt_array) ) {
        _libeep_evt_read_variant_array(f);
      }
      break;
  }
}
/*****************************************************************************/
static
void
_libeep_evt_read_epoch_descriptors(FILE *f) {
  int32_t i = 0;
  int32_t size = 0;

  if(fread(&size, sizeof(int32_t), 1, f) == 1) {

    _libeep_evt_log(evt_log_dbg, "%s: size: %i\n", __FUNCTION__, size);

    for(i=0;i<size;++i) {
      char * descriptor_name;
      char * descriptor_unit;

      _libeep_evt_log(evt_log_dbg, "%s: i: %i\n", __FUNCTION__, i);

      descriptor_name = _libeep_evt_read_string(f);
      _libeep_evt_log(evt_log_dbg, "%s: descriptor_name: %s\n", __FUNCTION__, descriptor_name);

      _libeep_evt_read_variant(f);

      descriptor_unit = _libeep_evt_read_string(f);
      _libeep_evt_log(evt_log_dbg, "%s: descriptor_unit: %s\n", __FUNCTION__, descriptor_unit);

      _libeep_evt_string_delete(descriptor_name);
      _libeep_evt_string_delete(descriptor_unit);
    }
  }
}
/*****************************************************************************/
static
libeep_evt_GUID_t *
_libeep_evt_read_GUID(FILE * f) {
  libeep_evt_GUID_t * rv = NULL;

  rv = libeep_evt_GUID_new();

  if(fread(rv, sizeof(libeep_evt_GUID_t), 1, f) == 1) {
    _libeep_evt_log(evt_log_dbg, "%s: GUID %i %i %i [%i %i %i %i %i %i %i %i] ....:\n", __FUNCTION__, rv->data1, rv->data2, rv->data3, rv->data4[0], rv->data4[1], rv->data4[2], rv->data4[3], rv->data4[4], rv->data4[5], rv->data4[6], rv->data4[7]);
  }
  return rv;
}
/*****************************************************************************/
static
void
_libeep_evt_read_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  libeep_evt_class_t * clss = NULL;

  if(fread(&ev->visible_id, sizeof(int32_t), 1, f) == 1) {
    ev->guid = _libeep_evt_read_GUID(f);
    clss = _libeep_evt_read_class(f);
    libeep_evt_class_delete(clss);
    ev->name = _libeep_evt_read_string(f);
    if(e->header.version >= 78) {
      ev->user_visible_name = _libeep_evt_read_string(f);
    }
    if(fread(&ev->type, sizeof(int32_t), 1, f) == 1) {
      if(fread(&ev->state, sizeof(int32_t), 1, f) == 1) {
        if(fread(&ev->original, sizeof(int8_t), 1, f) == 1) {
          if(fread(&ev->duration, sizeof(double), 1, f) == 1) {
            if(fread(&ev->duration_offset, sizeof(double), 1, f) == 1) {
              if(fread(&ev->time_stamp.date, sizeof(double), 1, f) == 1) {
                if(fread(&ev->time_stamp.fraction, sizeof(double), 1, f) == 1) {
                  _libeep_evt_log(evt_log_dbg, "%s: type: %i\n", __FUNCTION__, ev->type);
                  _libeep_evt_log(evt_log_dbg, "%s: state: %i\n", __FUNCTION__, ev->state);
                  _libeep_evt_log(evt_log_dbg, "%s: original: %i\n", __FUNCTION__, ev->original);
                  _libeep_evt_log(evt_log_dbg, "%s: duration: %g\n", __FUNCTION__, ev->duration);
                  _libeep_evt_log(evt_log_dbg, "%s: duration_offset: %g\n", __FUNCTION__, ev->duration_offset);
                  _libeep_evt_log(evt_log_dbg, "%s: time_stamp.date: %g\n", __FUNCTION__, ev->time_stamp.date);
                  _libeep_evt_log(evt_log_dbg, "%s: time_stamp.fraction: %g\n", __FUNCTION__, ev->time_stamp.fraction);
                  if(e->header.version >= 11 && e->header.version < 19) {
                    TODO_MARKER;
                  }
                  if(e->header.version >= 19) {
                    _libeep_evt_read_epoch_descriptors(f);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
/*****************************************************************************/
static
void
_libeep_evt_read_channel_info(FILE * f) {
  char * channel_active;
  char * channel_reference;

  channel_active = _libeep_evt_read_string(f);
  channel_reference = _libeep_evt_read_string(f);

  _libeep_evt_log(evt_log_dbg, "%s: channel_active: %s\n", __FUNCTION__, channel_active);
  _libeep_evt_log(evt_log_dbg, "%s: channel_reference: %s\n", __FUNCTION__, channel_reference);

  _libeep_evt_string_delete(channel_active);
  _libeep_evt_string_delete(channel_reference);
}
/*****************************************************************************/
static
void
_libeep_evt_read_epoch_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  _libeep_evt_read_event(f, e, ev);
  if(e->header.version < 33) {
    int32_t tmp;
    fread(&tmp, sizeof(int32_t), 1, f);
    _libeep_evt_log(evt_log_dbg, "%s: tmp: %s\n", __FUNCTION__, tmp);
  }
}
/*****************************************************************************/
static
void
_libeep_evt_read_event_marker(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  int32_t   show_amplitude = 0;
  int8_t    show_duration = 0;
  char    * description = NULL;

  _libeep_evt_read_event(f, e, ev);
  _libeep_evt_read_channel_info(f);
  description = _libeep_evt_read_string(f);
  if(e->header.version >= 35) {
    if(e->header.version >= 103) {
      fread(&show_amplitude, sizeof(int32_t), 1, f);
    } else {
      int8_t tmp;
      fread(&tmp, sizeof(int8_t), 1, f);
      show_amplitude = tmp;
    }
    fread(&show_duration, sizeof(int8_t), 1, f);
  }
  _libeep_evt_log(evt_log_dbg, "%s: show_amplitude: %i\n", __FUNCTION__, show_amplitude);
  _libeep_evt_log(evt_log_dbg, "%s: show_duration: %i\n", __FUNCTION__, show_duration);
  _libeep_evt_log(evt_log_dbg, "%s: description: %s\n", __FUNCTION__, description);

  _libeep_evt_string_delete(description);
}
/*****************************************************************************/
static
void
_libeep_evt_read_artefact_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  char * description = NULL;

  _libeep_evt_read_event(f, e, ev);
  _libeep_evt_read_channel_info(f);
  if(e->header.version >= 174) {
    description = _libeep_evt_read_string(f);
    _libeep_evt_log(evt_log_dbg, "%s: description: %s\n", __FUNCTION__, description);
  }

  _libeep_evt_string_delete(description);
}
/*****************************************************************************/
static
void
_libeep_evt_read_spike_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  _libeep_evt_read_event(f, e, ev);
  TODO_MARKER;
}
/*****************************************************************************/
static
void
_libeep_evt_read_seizure_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  _libeep_evt_read_event(f, e, ev);
  TODO_MARKER;
}
/*****************************************************************************/
static
void
_libeep_evt_read_sleep_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  _libeep_evt_read_event(f, e, ev);
  TODO_MARKER;
}
/*****************************************************************************/
static
void
_libeep_evt_read_rpeak_event(FILE * f, libeep_evt_t * e, libeep_evt_event_t * ev) {
  _libeep_evt_read_event(f, e, ev);
  TODO_MARKER;
}
/*****************************************************************************/
void
libeep_evt_header_print(const libeep_evt_header_t * h) {
  _libeep_evt_log(evt_log_inf, "header:\n");
  _libeep_evt_log(evt_log_inf, "  ctime: %i\n", h->ctime);
  _libeep_evt_log(evt_log_inf, "  mtime: %i\n", h->mtime);
  _libeep_evt_log(evt_log_inf, "  atime: %i\n", h->atime);
  _libeep_evt_log(evt_log_inf, "  version: %i\n", h->version);
  _libeep_evt_log(evt_log_inf, "  comp.mode: %i\n", h->compression_mode);
  _libeep_evt_log(evt_log_inf, "  encr.mode: %i\n", h->encryption_mode);
}
/*****************************************************************************/
void
libeep_evt_event_print(const libeep_evt_event_t * e) {
  _libeep_evt_log(evt_log_inf, "libeep_evt_event_t {\n");
  _libeep_evt_log(evt_log_inf, "  visible_id......... %i\n", e->visible_id);
  if(e->guid) {
    _libeep_evt_log(evt_log_inf, "  GUID............... %i %i %i [%i %i %i %i %i %i %i %i]\n", e->guid->data1, e->guid->data2, e->guid->data3, e->guid->data4[0], e->guid->data4[1], e->guid->data4[2], e->guid->data4[3], e->guid->data4[4], e->guid->data4[5], e->guid->data4[6], e->guid->data4[7]);
  }
  _libeep_evt_log(evt_log_inf, "  name............... %s\n", e->name);
  _libeep_evt_log(evt_log_inf, "  user_visible_name.. %s\n", e->user_visible_name);
  _libeep_evt_log(evt_log_inf, "  type............... %i\n", e->type);
  _libeep_evt_log(evt_log_inf, "  state.............. %i\n", e->state);
  _libeep_evt_log(evt_log_inf, "  original........... %i\n", e->original);
  _libeep_evt_log(evt_log_inf, "  duration........... %g\n", e->duration);
  _libeep_evt_log(evt_log_inf, "  duration_offset.... %g\n", e->duration_offset);
  _libeep_evt_log(evt_log_inf, "  timestamp.......... %8.8f / %8.8f\n", e->time_stamp.date, e->time_stamp.fraction);
  _libeep_evt_log(evt_log_inf, "}\n");
}
/******************************************************************************
 * internal function; read evt library
 *****************************************************************************/
static
void
_libeep_evt_read_library(FILE * f, libeep_evt_t * e) {
  char               * library_name;
  uint32_t             length;
  uint32_t             i;
  libeep_evt_class_t * clss;

  library_name = _libeep_evt_read_string(f);

  _libeep_evt_log(evt_log_dbg, "%s: library_name: %s\n", __FUNCTION__, library_name);

  _libeep_evt_string_delete(library_name);

  if(fread(&length, sizeof(uint32_t), 1, f) == 1) {
    _libeep_evt_log(evt_log_dbg, "%u library items\n", length);

    for(i=0;i<length;++i) {
      libeep_evt_event_t * ev = libeep_evt_event_new();
      clss = _libeep_evt_read_class(f);

      _libeep_evt_log(evt_log_dbg, "%s: i: %u class(%i, %s)\n", __FUNCTION__, i, clss->tag, clss->name);

      if(clss->name) {
        if (!strcmp(clss->name, "class dcEpochEvent_c"))         { _libeep_evt_read_epoch_event(f, e, ev); }
        else if (!strcmp(clss->name, "class dcEventMarker_c"))   { _libeep_evt_read_event_marker(f, e, ev); }
        else if (!strcmp(clss->name, "class dcArtefactEvent_c")) { _libeep_evt_read_artefact_event(f, e, ev); }
        else if (!strcmp(clss->name, "class dcSpikeEvent_c"))    { _libeep_evt_read_spike_event(f, e, ev); }
        else if (!strcmp(clss->name, "class dcSeizureEvent_c"))  { _libeep_evt_read_seizure_event(f, e, ev); }
        else if (!strcmp(clss->name, "class dcSleepEvent_c"))    { _libeep_evt_read_sleep_event(f, e, ev); }
        else if (!strcmp(clss->name, "class dcRPeakEvent_c"))    { _libeep_evt_read_rpeak_event(f, e, ev); }
        else {
          _libeep_evt_log(evt_log_err, "unknown class: %s\n", clss->name);
/*
          TODO_MARKER;
*/
        }
      }

      libeep_evt_class_delete(clss);
      _libeep_evt_event_list_add_back(e, ev);
    }
  }
}
/******************************************************************************
 * internal function; read evt file
 *****************************************************************************/
static
void
_libeep_evt_read_file(FILE * f, libeep_evt_t * e) {
  libeep_evt_class_t * clss;

  if(fread(&e->header, sizeof(libeep_evt_header_t), 1, f) != 1) {
    return;
  }

  /* libeep_evt_header_print(&e->header); */

  clss = _libeep_evt_read_class(f);
  if(clss && clss->tag==-1 && !strcmp("class dcEventsLibrary_c", clss->name)) {
    _libeep_evt_read_library(f, e);
  }
  libeep_evt_class_delete(clss);
}
/******************************************************************************
 * evt io
 *****************************************************************************/
libeep_evt_t *
libeep_evt_read(const char * filename) {
  libeep_evt_t * rv;
  FILE         * f;

  f = fopen(filename, "rb");
  if(f == NULL) {
    return NULL;
  }

  rv = libeep_evt_new();
  _libeep_evt_read_file(f, rv); 

  fclose(f);

  return rv;
}
