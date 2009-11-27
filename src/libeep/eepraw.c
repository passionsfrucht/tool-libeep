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

#include <string.h>

#include <eep/eepraw.h>

#ifdef COMPILE_RCS
char RCS_eepraw_h[] = RCS_EEPRAW_H;
char RCS_eepraw_c[] = "$RCSfile: eepraw.c,v $ $Revision: 2415 $";
#endif

#define SWAP32(x) \
        ((unsigned int)((((unsigned int)(x) & 0x000000ff) << 24) | \
                        (((unsigned int)(x) & 0x0000ff00) <<  8) | \
                        (((unsigned int)(x) & 0x00ff0000) >>  8) | \
                        (((unsigned int)(x) & 0xff000000) >> 24)))

#define SWAP16(x) \
        ((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                              (((unsigned short int)(x) & 0xff00) >> 8)))

int read_s32  (FILE *f, int *v)
{
#if EEP_BYTE_ORDER == EEP_LITTLE_ENDIAN
  return fread((char *) v, 4, 1, f);
#else
  if (!fread((char *) v, 4, 1, f)) return 0;
  *v = SWAP32(*v);
  return 1;
#endif
}

int read_u32  (FILE *f, unsigned int *v)
{
#if EEP_BYTE_ORDER == EEP_LITTLE_ENDIAN
  return fread((char *) v, 4, 1, f);
#else
  if (!fread((char *) v, 4, 1, f)) return 0;
  *v = SWAP32(*v);
  return 1;
#endif
}

void swrite_s32(char *s, int v)
{
  s[0] = (char) v;
  s[1] = (char) (v >> 8);
  s[2] = (char) (v >> 16);
  s[3] = (char) (v >> 24);
}

int write_s32 (FILE *f, int v)
{
  char out[4];
  swrite_s32(out, v);
  return fwrite((char *) out, 4, 1, f);
}

int write_u32 (FILE *f, unsigned int v)
{
  char out[4];
  swrite_s32(out, v);
  return fwrite((char *) out, 4, 1, f);
}

int read_u16 (FILE *f, int *v)
{
  unsigned char in[2];
  if (!fread((char *) in, 2, 1, f)) return 0;
  *v = (unsigned int) in[0] + (in[1] << 8);
  return 1;
}

int read_s16  (FILE *f, int *v)
{
  unsigned char in[2];
  if (!fread((char *) in, 2, 1, f)) return 0;

  /* negative value ? */
  if (in[1] & 0x80) {
    *v = -1 & ~0xffff;   /* lower 16 bit 0, other 1 */
    *v |= (int) in[0] + (in[1] << 8);
  }
  else {
    *v = (int) in[0] + (in[1] << 8);
  }
  return 1;
}

void swrite_s16(char *s, int v)
{
  s[0] = (char) v;
  s[1] = (char) (v >> 8);
}

int write_u16(FILE *f, int v)
{
  char out[2];
  swrite_s16(out, v);
  return fwrite((char *) out, 2, 1, f);
}

int write_s16(FILE *f, int v)
{
  char out[2];
  swrite_s16(out, v);
  return fwrite((char *) out, 2, 1, f);
}

/*
void c2dos_single(char *s, float v)
{
  char *x = (char *) &v;
#if EEP_BYTE_ORDER == EEP_LITTLE_ENDIAN
  v = SWAP32(v);
#endif
  s[0] = x[0];
  s[1] = x[1];
  s[2] = x[2];
  s[3] = x[3];
}
*/


int read_f32 (FILE *f, float *v)
{
#if EEP_FLOAT_ORDER == EEP_LITTLE_ENDIAN
  return fread((char *) v, 4, 1, f);
#else
  register char *tmp = (char *) v;
  register char c;
  if (!fread(tmp, 4, 1, f)) return 0;
  c = tmp[0]; tmp[0] = tmp[3]; tmp[3] = c;
  c = tmp[1]; tmp[1] = tmp[2]; tmp[2] = c;
  return 1;
#endif
}

int write_f32(FILE *f, float v)
{
  register char *tmp = (char *) &v;
#if EEP_FLOAT_ORDER == EEP_BIG_ENDIAN
  register char c;
  c = tmp[0]; tmp[0] = tmp[3]; tmp[3] = c;
  c = tmp[1]; tmp[1] = tmp[2]; tmp[2] = c;
#endif
  return fwrite(tmp, 4, 1, f);
}

int read_f64(FILE *f, double *v)
{
#if EEP_FLOAT_ORDER == EEP_LITTLE_ENDIAN
  return fread((char *) v, 8, 1, f);
#else
  register char *tmp = (char *) v;
  register char c;
  if (!fread(tmp, 8, 1, f)) return 0;
  c = tmp[0]; tmp[0] = tmp[7]; tmp[7] = c;
  c = tmp[1]; tmp[1] = tmp[6]; tmp[6] = c;
  c = tmp[2]; tmp[2] = tmp[5]; tmp[5] = c;
  c = tmp[3]; tmp[3] = tmp[4]; tmp[4] = c;
  return 1;
#endif
}

int write_f64(FILE *f, double v)
{
  register char *tmp = (char *) &v;
#if EEP_FLOAT_ORDER == EEP_BIG_ENDIAN
  register char c;
  c = tmp[0]; tmp[0] = tmp[7]; tmp[7] = c;
  c = tmp[1]; tmp[1] = tmp[6]; tmp[6] = c;
  c = tmp[2]; tmp[2] = tmp[5]; tmp[5] = c;
  c = tmp[3]; tmp[3] = tmp[4]; tmp[4] = c;
#endif
  return fwrite(tmp, 8, 1, f);
}

void swrite_f32  (char *s, float v)
{
  register char *tmp = (char *) &v;
#if EEP_FLOAT_ORDER == EEP_BIG_ENDIAN
  s[0] = tmp[3];
  s[1] = tmp[2];
  s[2] = tmp[1];
  s[3] = tmp[0];
#else
  memcpy(s, tmp, sizeof(float));
#endif
}

void sread_f32   (char *s, float *v)
{
  register char *tmp = (char *) v;
#if EEP_FLOAT_ORDER == EEP_BIG_ENDIAN
  tmp[0] = s[3];
  tmp[1] = s[2];
  tmp[2] = s[1];
  tmp[3] = s[0];
#else
  memcpy(tmp, s, sizeof(float));
#endif
}

void swrite_f64(char *s, double v)
{
  register char *tmp = (char *) &v;
#if EEP_FLOAT_ORDER == EEP_BIG_ENDIAN
  s[0] = tmp[7];
  s[1] = tmp[6];
  s[2] = tmp[5];
  s[3] = tmp[4];
  s[4] = tmp[3];
  s[5] = tmp[2];
  s[6] = tmp[1];
  s[7] = tmp[0];
#else
  memcpy(s, tmp, sizeof(double));
#endif
}

int vread_s16(FILE *f, sraw_t *buf, int n)
{
  register int j, status;
  register unsigned char *tmp = (unsigned char *) buf;
  
  status = fread(tmp, 2, n, f);
  if (status != n)
    return status;
  
  for (j = n - 1; j >= 0; j--) {
    buf[j] = ((int) tmp[2*j + 1] << 8) | tmp[2*j];
    if (buf[j] & 0x8000)
      buf[j] |= 0xffff0000;
  }
  return n;
}
  
int vwrite_s16(FILE *f, sraw_t *buf, int n)
{

  register int j;
  int  nr;
  register unsigned char *tmp = (unsigned char *) buf;

  for (j = 0; j < n; j++) {
    tmp[2*j] = (unsigned char) (buf[j]);
    tmp[2*j+1] = (unsigned char) (buf[j] >> 8);
  }
  nr = fwrite(tmp, 2, n, f);

  /* reestablish original byte order */
  for (j = 0; j < n; j++) {                                                     
    tmp[2*j] = (unsigned char) (buf[j]);                                       
    tmp[2*j+1] = (unsigned char) (buf[j] >> 8);                                
  }  

  return nr;
}

int vread_f32(FILE *f, float *buf, int n)
{
  register char *tmp = (char *) buf;

#if EEP_FLOAT_ORDER == EEP_LITTLE_ENDIAN
  return fread(tmp, 4, n, f);
#else
  
  int status = fread(tmp, 4, n, f);
  register int j;
  register char *w,c;

  if (status == n) {
    for (j = 0; j < n; j++) {
      w = &tmp[j*4];
      c = w[0]; w[0] = w[3]; w[3] = c;
      c = w[1]; w[1] = w[2]; w[2] = c;
    }
  }
  return status;
#endif
}
  
int vwrite_f32(FILE *f, float *buf, int n)
{
  register char *tmp = (char *) buf;

#if EEP_FLOAT_ORDER == EEP_BIG_ENDIAN
  register int j;
  int nr;
  register char *w,c;

  for (j = 0; j < n; j++) {
    w = &tmp[j*4];
    c = w[0]; w[0] = w[3]; w[3] = c;
    c = w[1]; w[1] = w[2]; w[2] = c;
  }
  nr = fwrite(tmp, 4, n, f);

  /* reestablish original byte order */
  for (j = 0; j < n; j++) {
    w = &tmp[j*4];
    c = w[0]; w[0] = w[3]; w[3] = c;
    c = w[1]; w[1] = w[2]; w[2] = c;
  }

  return nr; 
#endif

  return fwrite(tmp, 4, n, f);
}

int vread_s32(FILE *f, sraw_t *buf, int n)
{
  register int j, status;

  status = fread(buf, 4, n, f);
  if (status != n)
    return status;
  
#if EEP_BYTE_ORDER == EEP_LITTLE_ENDIAN
  return n;
#else
  for (j = n - 1; j >= 0; j--) {
    buf[j] = SWAP32(buf[j]);
  }
  return n;
#endif
}

