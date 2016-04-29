// matlab
#include "mex.h"
#include "matrix.h"
// libeep
#include <v4/eep.h>
///////////////////////////////////////////////////////////////////////////////
void
mexFunction (int nlhs, mxArray * plhs[], int nrhs, const mxArray * prhs[]) {
  mxArray * mx_version;

  if(nrhs!=0) {
    mexErrMsgTxt ("Invalid number of input arguments");
  }

  mx_version = mxCreateString(libeep_get_version());
  mxSetField(plhs[0], 0, "version", mx_version);
}
