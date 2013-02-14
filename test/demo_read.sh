#!/bin/bash

# assuming libeep was installed in /tmp/libeep
export LIBEEP_PREFIX=/tmp/libeep
export LD_LIBRARY_PATH=${LIBEEP_PREFIX}/lib:/usr/lib/jvm/java-6-sun-1.6.0.26/jre/lib/i386/xawt
export CLASSPATH=$(dirname $0):${LIBEEP_PREFIX}/lib/libeep.jar

javac $(dirname $0)/demo_read.java
java demo_read $*
