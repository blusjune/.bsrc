#!/bin/sh

_testlock=".testlock.tpcc";
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

echo ">>> Start cache_stat for TPC-C BMT $_tid $_tstamp";
_iterstop_flag=".testlock.cache_stat.stop";
while [ ! -f $_iterstop_flag ]; do
	velomgmt -adv -once 2>&1 | tee -a ./log/${_tid}/${_tstamp}.${_tid}.cache_stat.log;
done

