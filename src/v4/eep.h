#ifndef __libeep_v4_eep_h__
#define __libeep_v4_eep_h__

// system
#include <time.h>
#include <stdint.h>

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
 * @param channel_labels labels for the individual channels. might be NULL. in this case the labels will be auto-generated
 * @param channel_units unit strings for the individual channels. might be NULL. in this case the unit for all channels will be "uV"
 * @return -1 on error, handle otherwise
 */
int libeep_write_cnt(const char *filename, int rate, int nchan, const char **channel_labels, const char **channel_units);
/**
 * @brief close data file
 * @param handle handle to open
 */
void libeep_close(int handle);
/**
 * @brief get the number of channels
 */
int libeep_get_channel_count(int handle);
/**
 * @brief get a channel label
 * @param handle handle to open
 * @param index channel index
 * @return a channel label(do not free this string)
 */
const char * libeep_get_channel_label(int handle, int index);
/**
 * @brief get a channel unit
 * @return a channel unit(do not free this string)
 */
const char * libeep_get_channel_unit(int handle, int index);
/**
 * @brief get a channel reference
 * @return a channel reference(do not free this string)
 */
const char * libeep_get_channel_reference(int handle, int index);
/**
 * @brief get the channel scaling
 * @return scaling of the channel name
 */
float libeep_get_channel_scale(int handle, int index);
/**
 * @brief get the channel index
 * @return an index to the channel name
 */
int libeep_get_channel_index(int handle, const char *label);
/**
 * @brief get the sample frequency
 * @return sample frequency in Hz
 */
int libeep_get_sample_frequency(int handle);
/**
 * @brief get the number of samples
 * @return number of samples
 */
long libeep_get_sample_count(int handle);
/**
 * @brief get data samples
 * @return dynamically allocated array of samples or NULL on failure(Result should be freed with a call to libeep_free_samples)
 */
float * libeep_get_samples(int handle, long from, long to);
/**
* @brief deallocates the buffer returned by libeep_get_samples
* @param data pointer to float array
*/
void libeep_free_samples(float *);
/**
 * @brief add data samples
 * @param handle handle
 * @param data pointer to float array
 * @param n number of items in array
 */
void libeep_add_samples(int handle, const float *data, int n);
/**
* @brief add data samples
* @param handle handle
* @param data pointer to int array
* @param n number of items in array
*/
void libeep_add_raw_samples(int handle, const int32_t *data, int n);
/**
* @brief get data samples
* @return dynamically allocated array of samples or NULL on failure(Result should be freed with a call to libeep_free_raw_samples)
*/
int32_t * libeep_get_raw_samples(int handle, long from, long to);
/**
* @brief deallocates the buffer returned by libeep_get_raw_samples
* @param data pointer to int32_t array
*/
void libeep_free_raw_samples(int32_t *buffer);
/**
* @brief retrieves the start time of the recording
* @param handle handle
* @returns time_t(0) if the file does not contain recording info
*/
time_t libeep_get_start_time(int handle);
/**
* @brief sets the start time of the recording
* @param handle handle
* @param start_time the desired start time stamp
*/
void libeep_set_start_time(int handle, time_t start_time);
/**
* @brief retrieves information about the hospital the recording was made
* @param handle handle
* @returns the name of the hospital. the returned pointer shall not be free'd
*/
const char *libeep_get_hospital(int handle);
/**
* @brief sets information about the hospital the recording was made
* @param handle handle
*/
void libeep_set_hospital(int handle, const char *value);
/**
* @brief retrieves information about the name of the test this recording belongs to
* @param handle handle
* @returns the name test. the returned pointer shall not be free'd
*/
const char *libeep_get_test_name(int handle);
/**
* @brief sets information about the name of the test this recording belongs to
* @param handle handle
*/
void libeep_set_test_name(int handle, const char *value);
/**
* @brief retrieves information about the test serial number
* @param handle handle
* @returns the serial number of the test. the returned pointer shall not be free'd
*/
const char *libeep_get_test_serial(int handle);
/**
* @brief sets information about the test serial number
* @param handle handle
*/
void libeep_set_test_serial(int handle, const char *value);
/**
* @brief retrieves information about the physician responsible for the recording
* @param handle handle
* @returns the name of the physician. the returned pointer shall not be free'd
*/
const char *libeep_get_physician(int handle);
/**
* @brief sets information about the physician responsible for the recording
* @param handle handle
*/
void libeep_set_physician(int handle, const char *value);
/**
* @brief retrieves information about the technician responsible for the recording
* @param handle handle
* @returns the name of the technician. the returned pointer shall not be free'd
*/
const char *libeep_get_technician(int handle);
/**
* @brief sets information about the technician responsible for the recording
* @param handle handle
*/
void libeep_set_technician(int handle, const char *value);
/**
* @brief retrieves information about the acquisition hardware
* @param handle handle
* @returns the name of the hardware. the returned pointer shall not be free'd
*/
const char *libeep_get_machine_make(int handle);
/**
* @brief sets information about the acquisition hardware
* @param handle handle
*/
void libeep_set_machine_make(int handle, const char *value);
/**
* @brief retrieves information about the model of the acquisition hardware
* @param handle handle
* @returns the model. the returned pointer shall not be free'd
*/
const char *libeep_get_machine_model(int handle);
/**
* @brief sets information about the model of the acquisition hardware
* @param handle handle
*/
void libeep_set_machine_model(int handle, const char *value);
/**
* @brief retrieves information about the serial number of the acquisition hardware
* @param handle handle
* @returns the serial number. the returned pointer shall not be free'd
*/
const char *libeep_get_machine_serial_number(int handle);
/**
* @brief sets information about the serial number of the acquisition hardware
* @param handle handle
*/
void libeep_set_machine_serial_number(int handle, const char *value);
/**
* @brief retrieves information about the name of the patient
* @param handle handle
* @returns the name of the patient. the returned pointer shall not be free'd
*/
const char *libeep_get_patient_name(int handle);
/**
* @brief sets information about the name of the patient
* @param handle handle
*/
void libeep_set_patient_name(int handle, const char *value);
/**
* @brief retrieves information about the identification number for this patient
* @param handle handle
* @returns the patient id. the returned pointer shall not be free'd
*/
const char *libeep_get_patient_id(int handle);
/**
* @brief sets information about the identification number for this patient
* @param handle handle
*/
void libeep_set_patient_id(int handle, const char *value);
/**
* @brief retrieves information about the address of the patient
* @param handle handle
* @returns the address of the patient. the returned pointer shall not be free'd
*/
const char *libeep_get_patient_address(int handle);
/**
* @brief sets information about the address of the patient
* @param handle handle
*/
void libeep_set_patient_address(int handle, const char *value);
/**
* @brief retrieves information about the phone number of the patient
* @param handle handle
* @returns the patient's phone number. the returned pointer shall not be free'd
*/
const char *libeep_get_patient_phone(int handle);
/**
* @brief sets information about the phone number of the patient
* @param handle handle
*/
void libeep_set_patient_phone(int handle, const char *value);
/**
* @brief retrieves information about any comments about this recording
* @param handle handle
* @returns the comments. the returned pointer shall not be free'd
*/
const char *libeep_get_comment(int handle);
/**
* @brief sets information about any comments about this recording
* @param handle handle
*/
void libeep_set_comment(int handle, const char *value);
/**
* @brief retrieves information about the sex of the patient
* @param handle handle
* @returns the sex of the patient. Possible values are 'M' for male and 'F' for female
*/
char libeep_get_patient_sex(int handle);
/**
* @brief sets information about the sex of the patient
* @param handle handle
*/
void libeep_set_patient_sex(int handle, char value);
/**
* @brief retrieves information about the handedness of the patient
* @param handle handle
* @returns the handedness of the patient. Possible values are 'L' for left, 'R' for right and 'M' for mixed
*/
char libeep_get_patient_handedness(int handle);
/**
* @brief sets information about the handedness of the patient
* @param handle handle
*/
void libeep_set_patient_handedness(int handle, char value);

// TODO: day of birth

/**
* @brief inserts a trigger into the file
* @param handle handle
* @param sample trigger insertion position
* @param code trigger label
*/
int libeep_add_trigger(int handle, uint64_t sample, const char *code);
/**
* @brief returns the count of all triggers
* @param handle handle
*/
int libeep_get_trigger_count(int handle);
/**
* @brief returns the label of the trigger at certain position in the trigger table
* @param handle handle
* @param handle trigger index in the trigger table
* @param sample the sample at which the trigger is positioned
*/
const char *libeep_get_trigger(int handle, int idx, uint64_t *sample);
/**
 * @brief get zero offset(averages only)
 * @return offset of sample where event occurred
 */
long libeep_get_zero_offset(int handle);
/**
 * @brief get condition label(averages only)
 * @return condition label(do not free this string)
 */
const char * libeep_get_condition_label(int handle);
/**
 * @brief get condition color(averages only)
 * @return condition color(do not free this string)
 */
const char * libeep_get_condition_color(int handle);
/**
 * @brief get total number of trials(averages only)
 * @return total number of trials
 */
long libeep_get_trials_total(int handle);
/**
 * @brief get averaged number of trials(averages only)
 * @return averaged number of trials
 */
long libeep_get_trials_averaged(int handle);

#endif
