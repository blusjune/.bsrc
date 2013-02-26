#!/bin/sh


_tstamp=$(tstamp);
_testlock="/tmp/.testlock.cache_bmt";
if [ "X$_tid" = "X" ]; then
	read -p "Test ID (tid): " _tid;
fi
echo ">>> $_tid#$_tstamp";
echo "$_tid#$_tstamp" > $_testlock;

./.cache_stat.velobit_hypercache.sh
