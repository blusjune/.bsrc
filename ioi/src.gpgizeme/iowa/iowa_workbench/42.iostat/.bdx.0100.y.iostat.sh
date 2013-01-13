#!/bin/sh

##tstamp: 20121116_142046

_testlock=".testlock.iowa";
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
	##
	## case-specific code {
	##

	#_iostat_timelimit=3600;
	if [ "X$_iostat_timelimit" = "X" ]; then
		read -p "time limit for iostat run (sec) : " _iostat_timelimit;
	fi
	_devlist_file=".devlist";
	if [ -f $_devlist_file ]; then
		_devlist=$(cat $_devlist_file | grep -v '^#');
		echo "_devlist: $_devlist";
	else
		read -p "list of devices to be monitored (e.g., '/dev/sdc1 /dev/sdd1') : " _devlist;
	fi
	echo ">>> start iostat for $_devlist ($_tid $_tstamp)";
	iostat -xt $_devlist 1 $_iostat_timelimit | tee ./log/${_tid}/${_tstamp}.${_tid}.iostat.log;
	#iostat loop0 sdc1 -xt 1 3600 | tee ${_trd}/iostat.${_trd_radix}.log;	# _target_dev="loop0 sdc1";

	##
	## } case-specific code
	##
fi
