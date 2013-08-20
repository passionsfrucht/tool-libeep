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

#include <math.h>
#include "mex.h"		/* Matlab specific  */
#include "matrix.h"		/* Matlab specific  */
#include <eep/eepmisc.h>		/* MPI-ANT specific */
#include <cnt/cnt.h>		/* MPI-ANT specific */

#define NAME "read_eep_cnt"

void
mexFunction (int nlhs, mxArray * plhs[], int nrhs, const mxArray * prhs[])
{

  /* these variables are MPI-ANT specific for reading the data */
  char filename[256];
  int chan, sample, sb, se, length, chanc, samplec;
  float period;
  int status;
  FILE *fp;
  sraw_t *buf;
  eeg_t *hdr;

  /* these variables are Matlab specific for interfacing */
  double *ptr;
  const int dims[] = {1, 1};
  mxArray *label;
  mxArray *rate;
  mxArray *npnt;
  mxArray *nchan;
  mxArray *nsample;
  mxArray *time;
  mxArray *data;

  const int nfields = 7;
  const char *field_names[] = {
    "label",                     /* label			*/
    "rate",                      /* 1/(period)			*/
    "npnt",                      /* nsample of this segment	*/
    "nchan",                     /* chanc			*/
    "nsample",                   /* nsample of the whole data	*/
    "time",                      /* 				*/
    "data"};                     /* data			*/

  if (nrhs!=3)
    mexErrMsgTxt ("Invalid number of input arguments");

  mxGetString(prhs[0], filename, 256);
  sb = (int)(mxGetScalar(prhs[1]) + 0.5) - 1;
  se = (int)(mxGetScalar(prhs[2]) + 0.5) - 1;
  length = se-sb+1;

  /* open the data file */
 if ((fp = eepio_fopen(filename, "rb"))==NULL)
    mexErrMsgTxt ("Could not open file");

  /* read header information */
  hdr = eep_init_from_file(filename, fp, &status);
  if ((hdr == NULL) || (status!=CNTERR_NONE))
    mexErrMsgTxt ("Error reading header from file");

  chanc   = eep_get_chanc(hdr);
  period  = eep_get_period(hdr);
  samplec = eep_get_samplec(hdr);

  if (sb<0)
    mexErrMsgTxt ("Begin sample should be 1 or larger");

  if (se<sb)
    mexErrMsgTxt ("End sample should be similar to, or larger than the begin sample");

  if (chanc<1)
    mexErrMsgTxt ("Invalid number of channels in the data");

  if (se>samplec)
    mexErrMsgTxt ("End sample should be less than the number of samples in the data");

  /* rate      = mxCreateDoubleMatrix(1,1,mxREAL); *mxGetPr(rate     ) = (double)1/period; */
  rate      = mxCreateDoubleMatrix(1,1,mxREAL); *mxGetPr(rate     ) = (double)eep_get_rate(hdr);
  npnt      = mxCreateDoubleMatrix(1,1,mxREAL); *mxGetPr(npnt     ) = (double)length;
  nchan     = mxCreateDoubleMatrix(1,1,mxREAL); *mxGetPr(nchan    ) = (double)chanc;
  nsample   = mxCreateDoubleMatrix(1,1,mxREAL); *mxGetPr(nsample  ) = (double)samplec;
  data      = mxCreateDoubleMatrix(chanc,length,mxREAL);
  time      = mxCreateDoubleMatrix(1,length,mxREAL);
  label     = mxCreateCellMatrix(chanc,1);

  ptr = mxGetPr(time);
  for (sample=0; sample<length; sample++)
    ptr[sample] = (double)1000*(sb+sample)*period;

  for (chan=0; chan<chanc; chan++)
    mxSetCell(label,chan,mxCreateString(eep_get_chan_label(hdr, chan)));

  /* allocate memory for the data and read it from file */
  buf = (sraw_t *)malloc(CNTBUF_SIZE(hdr, chanc));
  ptr = mxGetPr(data);
  eep_seek(hdr, DATATYPE_EEG, sb, 0);
  for (sample=0; sample<length; sample++)
    if (eep_read_sraw(hdr, DATATYPE_EEG, buf, 1) != CNTERR_NONE)
      mexErrMsgTxt ("Error reading raw data from file");
    else
      for (chan=0; chan<chanc; chan++)
        ptr[sample*chanc+chan] = eep_get_chan_scale(hdr, chan) * buf[chan];

  /* create the struct array with dimensions 1x1 */
  plhs[0] = mxCreateStructArray(2, dims, nfields, field_names);

  /* fill the struct array with the variables */
  mxSetField(plhs[0], 0, "rate",      rate     );
  mxSetField(plhs[0], 0, "npnt",      npnt     );
  mxSetField(plhs[0], 0, "nchan",     nchan    );
  mxSetField(plhs[0], 0, "nsample",   nsample  );
  mxSetField(plhs[0], 0, "data",      data     );
  mxSetField(plhs[0], 0, "time",      time     );
  mxSetField(plhs[0], 0, "label",     label    );

  /* close the file */
  eep_free(hdr);
  eepio_fclose(fp);
  free(buf);

  return;
}
