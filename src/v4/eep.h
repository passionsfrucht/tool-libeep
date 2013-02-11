#ifndef __libeep_v4_eep_h__
#define __libeep_v4_eep_h__

/**
 * v4 of the libeep header provides a simplified interface to the libeep
 * project. It exposes no details of the internal structure
 *****/

/**
 * init library
 */
void libeep_init();
/**
 * exit library
 */
void libeep_exit();
/**
 * get version
 * @return version, do not free this string
 */
char * libeep_get_version();
/**
 * open file for reading
 * @return -1 on error, handle otherwise
 */
int libeep_read(const char *);
/**
 * close data file
 */
void libeep_close(int);
/**
 * get the number of channels
 */
int libeep_get_channel_count(int);
/**
 * get a channel name
 */
char * libeep_get_channel_label(int, int);
/**
 * get a channel name
 */
char * libeep_get_channel_unit(int, int);
/**
 * get a channel name
 */
char * libeep_get_channel_reference(int, int);
/**
 * get the sample frequency
 */
float libeep_get_sample_frequency(int);
/**
 * get the number of samples
 */
long libeep_get_sample_count(int);
/**
 * get data samples
 * @return dynamically allocated array of samples or NULL on failure. Result should be free'd.
 */
float * libeep_get_samples(int, long, long);

#endif
