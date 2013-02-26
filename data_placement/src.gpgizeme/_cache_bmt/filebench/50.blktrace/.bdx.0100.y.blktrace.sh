#!/bin/sh

_testlock=".testlock.filebench";

_trd="$(cat $_testlock)";
_trd_radix="$(basename $_trd)";
if [ "X$_trd_radix" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (blktrace)";
	exit 0;
fi

#bsc.bio.trace /dev/loop0 | tee ${_trd}/blktrace.${_trd_radix}.log;
bsc.bio.trace /dev/sdb1 | tee ${_trd}/blktrace.${_trd_radix}.log;
