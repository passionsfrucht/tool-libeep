#! /bin/sh

function bail() {
  echo "error running: $*"
  exit -1
}
function do_run() {
  echo "-------------------------------------------------------------------------------"
  echo "-- $*"
  $* || bail "$*"
}

do_run libtoolize --copy --force
do_run aclocal
do_run autoconf
do_run automake --add-missing --copy
