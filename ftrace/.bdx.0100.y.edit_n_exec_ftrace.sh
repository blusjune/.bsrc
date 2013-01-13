#!/bin/sh

##.bdx.0100.y.edit_n_exec_ftrace.sh
##tstamp: 20121206_160250

_tracelog="log/tracelog.$(tstamp)";

bsc.krnl.ftrace.filter_update;

./testcmd.sh

cat tracing/trace > $_tracelog;
