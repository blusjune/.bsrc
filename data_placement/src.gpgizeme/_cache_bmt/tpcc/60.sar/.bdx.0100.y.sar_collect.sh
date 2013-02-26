#!/bin/sh
##tstamp: 20121023_023515


_testlock=".testlock.cache_bmt";
if [ ! -f $_testlock ]; then
	echo "$_testlock does not exist";
	exit 1;
fi
_tid="$(cat $_testlock | sed -e 's/\(.*\)#.*/\1/g')";
_tstamp="$(cat $_testlock | sed -e 's/.*#\(.*\)/\1/g')";
if [ "X$_tid" = "X" -o "X$_tstamp" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (sar)";
	exit 2;
fi
if [ ! -d log/${_tid} ]; then
	mkdir -p log/${_tid};
fi

echo ">>> Start 'sar (system activity report)' for TPC-C BMT $_tid $_tstamp";

(cd ./log/${_tid}; _session_id="${_tstamp}.${_tid}" bsc.sar.collect;)
