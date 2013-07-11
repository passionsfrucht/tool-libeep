#ifndef __libeep_v4_eep_h__
#define __libeep_v4_eep_h__

/**
 * @file v4/eep.h
 * @brief v4 of the libeep header provides a simplified interface to the libeep
 * project. It exposes no details of the internal structure
 *****/

/**
 * @brief init library
 */
void libeep_init();
/**
 * @brief exit library
 */
void libeep_exit();
/**
 * @brief get library version
 * @return version(do not free this string)
 */
const char * libeep_get_version();
/**
 * @brief open file for reading
 * @param filename the filename to the CNT or AVR to open
 * @return -1 on error, handle otherwise
 */
int libeep_read(const char *filename);
/**
 * @brief open cnt file for writing
 * @param filename the filename to the CNT or AVR to open
 * @param rate the sampling rate(in Hz)
 * @param nchan the number of channels
 * @return -1 on error, handle otherwise
 */
int libeep_write_cnt(const char *filename, int rate, int nchan);
/**
 * @brief close data file
 * @param handle handle to open
 */
void libeep_close(int handle);
/**
 * @brief get the number of channels
 */
int libeep_get_channel_count(int);
/**
 * @brief get a channel label
 * @return a channel label(do not free this string)
 */
const char * libeep_get_channel_label(int, int);
/**
 * @brief get a channel unit
 * @return a channel unit(do not free this string)
 */
const char * libeep_get_channel_unit(int, int);
/**
 * @brief get a channel reference
 * @return a channel reference(do not free this string)
 */
const char * libeep_get_channel_reference(int, int);
/**
 * @brief get the channel scaling
 * @return scaling of the channel name
 */
float libeep_get_channel_scale(int, int);
/**
 * @brief get the channel index
 * @return an index to the channel name
 */
int libeep_get_channel_index(int, const char *);
/**
 * @brief get the sample frequency
 * @return sample frequency in Hz
 */
float libeep_get_sample_frequency(int);
/**
 * @brief get the number of samples
 * @return number of samples
 */
long libeep_get_sample_count(int);
/**
 * @brief get data samples
 * @return dynamically allocated array of samples or NULL on failure(Result should be free'd)
 */
float * libeep_get_samples(int, long, long);
/**
 * @brief add data samples
 * @param handle handle
 * @param data pointer to float array
 * @param n number of items in array
 */
void libeep_add_samples(int handle, const float *data, int n);
/**
 * @brief get zero offset(averages only)
 * @return offset of sample where event occurred
 */
long libeep_get_zero_offset(int);
/**
 * @brief get condition label(averages only)
 * @return condition label(do not free this string)
 */
const char * libeep_get_condition_label(int);
/**
 * @brief get condition color(averages only)
 * @return condition color(do not free this string)
 */
const char * libeep_get_condition_color(int);
/**
 * @brief get total number of trials(averages only)
 * @return total number of trials
 */
long libeep_get_trials_total(int);
/**
 * @brief get averaged number of trials(averages only)
 * @return averaged number of trials
 */
long libeep_get_trials_averaged(int);

#endif
