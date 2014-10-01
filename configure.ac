AC_INIT(libeep, 3.3.172)
AM_INIT_AUTOMAKE([subdir-objects silent-rules])
AC_CANONICAL_HOST

################
# version info #
################
AC_SUBST(LIBEEP_VERSION, [$VERSION])
AC_DEFINE(LIBEEP_VERSION, [VERSION])
PACKAGE_LIBRARY_VERSION=6:172:3
AC_SUBST(PACKAGE_LIBRARY_VERSION)

#######################
# checks for programs #
#######################
AC_PROG_CC
AC_PROG_LEX
AC_PROG_LIBTOOL
AC_PROG_INSTALL

###########################
# check configure options #
###########################
AC_ARG_ENABLE(matlab,        [  --enable-matlab          build matlab importers], condmex=yes, condmex=no)
AC_ARG_ENABLE(test,          [  --enable-test            build test utils      ], condtest=yes, condtest=no)

#################
# check enables #
#################
AM_CONDITIONAL(CONDMEX, test x$condmex = xyes)
AM_CONDITIONAL(CONDTEST, test x$condtest = xyes)

################
# check matlab #
################
if test x$condmex = xyes ; then
  sinclude(mex/matlab.m4)
  sinclude(mex/mexopt.m4)
  sinclude(mex/mex.m4)
  sinclude(mex/mexext.m4)

  AX_REQUIRE_MATLAB
  AX_PATH_MEX
  AX_MEXEXT
fi

###################
# generate output #
###################
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES(libeep-config, chmod +x libeep-config)
AC_OUTPUT([Makefile src/Makefile mex/Makefile test/Makefile libeep.pc])

##############
# debug info #
##############
echo ----------------------------------------------------------------------
echo PACKAGE.................. $PACKAGE
echo VERSION.................. $VERSION
echo PACKAGE_LIBRARY_VERSION.. $PACKAGE_LIBRARY_VERSION
echo
echo prefix....: $prefix
echo host......: $host
echo condtest..: $condtest
echo condmex...: $condmex
if test x$condmex = xyes ; then
  echo MEX......: $MEX
  echo MEXEXT...: $MEXEXT
  echo MEXOPTS..: $MEXOPTS
  echo MEXFLAGS.: $MEXFLAGS
fi
