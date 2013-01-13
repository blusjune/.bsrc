#!/bin/sh

_testlock=".testlock.filebench";
_iterstop_flag="/tmp/iter.stop";

_trd="$(cat $_testlock)";
_trd_radix="$(basename $_trd)";
if [ "X$_trd_radix" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (cache_stat)";
	exit 0;
fi
_logfile="cache_stat.${_trd_radix}.log";

while [ ! -f $_iterstop_flag ]; do
	velomgmt -adv -once 2>&1 | tee -a ${_trd}/cache_stat.${_trd_radix}.log;
done
