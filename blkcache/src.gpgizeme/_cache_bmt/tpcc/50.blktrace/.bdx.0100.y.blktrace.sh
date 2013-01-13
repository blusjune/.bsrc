#!/bin/sh

_testlock=".testlock.cache_bmt";
if [ ! -f $_testlock ]; then
	echo "$_testlock does not exist";
	exit 1;
fi
_tid="$(cat $_testlock | sed -e 's/\(.*\)#.*/\1/g')";
_tstamp="$(cat $_testlock | sed -e 's/.*#\(.*\)/\1/g')";
if [ "X$_tid" = "X" -o "X$_tstamp" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (blktrace)";
	exit 2;
fi
if [ ! -d log/${_tid} ]; then
	mkdir -p log/${_tid};
fi

if [ -f ./log/${_tid}/${_tstamp}.${_tid}.blktrace.log ]; then
	echo ">>> blktrace log is already being generated";
	exit 0;
else
	echo ">>> start blktrace for TPC-C BMT $_tid $_tstamp";
#	bsc.bio.trace /dev/sdd1 | tee ./log/${_tid}/${_tstamp}.${_tid}.blktrace.log;
	blktrace -d /dev/sdd -o - | blkparse -i - | tee ./log/${_tid}/${_tstamp}.${_tid}.blktrace.log;
fi
