#!/bin/sh

_testlock=".testlock.cache_bmt";
if [ ! -f $_testlock ]; then
	echo "$_testlock does not exist";
	exit 1;
fi
_tid="$(cat $_testlock | sed -e 's/\(.*\)#.*/\1/g')";
_tstamp="$(cat $_testlock | sed -e 's/.*#\(.*\)/\1/g')";
if [ "X$_tid" = "X" -o "X$_tstamp" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (iostat)";
	exit 2;
fi
if [ ! -d log/${_tid} ]; then
	mkdir -p log/${_tid};
fi

if [ "X${_target_dev}" = "X" ]; then
	read -p ">>> target devices (e.g., '/dev/loop0 /dev/sdc1' or 'loop0 sdc1'): " _target_dev;
fi

iostat $_target_dev -xt 1 36000 | tee ./log/${_tid}/${_tstamp}.${_tid}.iostat.log;
