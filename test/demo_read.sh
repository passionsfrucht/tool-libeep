#!/bin/bash

export LD_LIBRARY_PATH=/tmp/libeep/lib:/usr/lib/jvm/java-6-openjdk-i386/jre/lib/i386/xawt
export CLASSPATH=$(dirname $0):/tmp/libeep/lib/libeep.jar

javac $(dirname $0)/demo_read.java
java demo_read $*
