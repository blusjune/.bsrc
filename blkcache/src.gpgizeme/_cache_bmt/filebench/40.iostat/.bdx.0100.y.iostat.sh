#!/bin/sh

_testlock=".testlock.filebench";

_trd="$(cat $_testlock)";
_trd_radix="$(basename $_trd)";
if [ "X$_trd_radix" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (iostat)";
	exit 0;
fi

if [ "X${_target_dev}" = "X" ]; then
	read -p ">>> target devices (e.g., '/dev/loop0 /dev/sdc1' or 'loop0 sdc1'): " _target_dev;
fi

#iostat loop0 sdc1 -xt 1 3600 | tee ${_trd}/iostat.${_trd_radix}.log;	# _target_dev="loop0 sdc1";
iostat $_target_dev -xt 1 3600 | tee ${_trd}/iostat.${_trd_radix}.log;
