/********************************************************************************
 *                                                                              *
 * this file is part of:                                                        *
 * libeep, the project for reading and writing avr/cnt eeg and related files    *
 *                                                                              *
 ********************************************************************************
 *                                                                              *
 * LICENSE:Copyright (c) 2003-2009,                                             *
 * Advanced Neuro Technology (ANT) B.V., Enschede, The Netherlands              *
 * Max-Planck Institute for Human Cognitive & Brain Sciences, Leipzig, Germany  *
 *                                                                              *
 ********************************************************************************
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU Lesser General Public License as published by  *
 * the Free Software Foundation; either version 3 of the License, or            *
 * (at your option) any later version.                                          *
 *                                                                              *
 * This library is distributed WITHOUT ANY WARRANTY; even the implied warranty  *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU Lesser General Public License for more details.                          *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this program. If not, see <http://www.gnu.org/licenses/>          *
 *                                                                              *
 *******************************************************************************/

#ifndef EEPIO_H
#define EEPIO_H
#define RCS_EEPIO_H "$RCSfile: eepio.h,v $ $Revision: 2415 $"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <eep/eepmisc.h>
#include <eep/eepraw.h>

#if !defined(WIN32) || defined(__CYGWIN__)
typedef long long __int64;
#endif


/* 
  cfg line preprocessing
  remove whitespaces, cut comments starting with CFG_SEP
  changes are made in-place
  
  return: pointer to preprocessed line (maybe a pointer to empty string)
*/
#define CFG_SEP ';'

char *cfg_line_norm(char *line);
char *cfg_line_norm_cs(char *line);


/* ------------------------------------------------------------------- 
   terminal output
    log:   normal output: parameters, statistics...
           (allows global disable, stderr/stdout duplication)
    debug: additional infos as internal tables, states...
           (global on / off switch available)
    status:error messages, warnings
    
    error: same like status + clear + exit
    
    sys...:add perror output to status message
*/

/* global flags to control log func details */
extern int eepio_quiet;
extern int eepio_doslf; /* can be set but is ignored */
extern int eepio_log;

extern int EEPDebug;
extern int EEPBar;
extern int eepmess_log;
#define MESSORIGIN_SIZE 32
extern char messorigin[MESSORIGIN_SIZE];

int  eepstdout(const char *fmt, ...);
int  eepstderr(const char *fmt, ...);

void eeplog    (const char *fmt, ...);
void eepdebug  (const char *fmt, ...);
void eepstatus (const char *fmt, ...);
void sysstatus (const char *fmt, ...);
void eeperror  (const char *fmt, ...);
void syserror  (const char *fmt, ...);

/* 
  the eepmess.h *open funcs maintain a list of "autoremove" files here
  (the *error funcs need to know about this list for cleanup)
*/
extern int    ar_filec;
extern FILE **ar_file;
extern char **ar_filename;
void arv_fclear(void);

/* -------------------------------------------------------------------
   progress indicator bar
*/
void init_eep_bar(slen_t total);
void show_eep_bar(slen_t current);
void free_eep_bar(void);

/* A function to print a text wrapped at len characters */
void eep_print_wrap(FILE* out, const char* text, int len);

/****************************************************
 * swap 8 bytes in long long on big endian machines *
 ****************************************************/
__int64 eep_byteswap_8_safe(__int64);
/***********************************************
 * swap 4 bytes in long on big endian machines *
 ***********************************************/
long eep_byteswap_4_safe(long);
/************************************************
 * swap 4 bytes in short on big endian machines *
 ************************************************/
short eep_byteswap_2_safe(short);
/**************************
 * swap 8 bytes in double *
 **************************/
double eep_byteswap_8_double_safe(double arg);
  
#endif
