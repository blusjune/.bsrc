#!/bin/sh

##tstamp: 20121116_142046

_this_prog="blktrace";
_testlock=".testlock.iowa";
if [ ! -f $_testlock ]; then
	echo "$_testlock does not exist";
	exit 1;
fi
_tid="$(cat $_testlock | sed -e 's/\(.*\)#.*/\1/g')";
_tstamp="$(cat $_testlock | sed -e 's/.*#\(.*\)/\1/g')";
if [ "X$_tid" = "X" -o "X$_tstamp" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program ($_this_prog)";
	exit 2;
fi
if [ ! -d log/${_tid} ]; then
	mkdir -p log/${_tid};
fi

if [ -f ./log/${_tid}/${_tstamp}.${_tid}.$_this_prog.log ]; then
	echo ">>> $_this_prog log is already being generated";
	exit 0;
else
	##
	## case-specific code {
	##

	_devlist_file=".devlist";
	if [ -f $_devlist_file ]; then
		_devlist=$(cat $_devlist_file | grep -v '^#');
		echo "_devlist: $_devlist";
	else
		read -p "list of devices to be traced (e.g., '/dev/sdc1 /dev/sdd1') : " _devlist;
	fi
	echo ">>> start $_this_prog $_devlist ($_tid $_tstamp)";
	blktrace $_devlist -o - | blkparse -i - -d ./log/${_tid}/${_tstamp}.${_tid}.blktrace.bin | tee ./log/${_tid}/${_tstamp}.${_tid}.blktrace.log;
#	bsc.bio.trace /dev/sdd1 | tee ./log/${_tid}/${_tstamp}.${_tid}.blktrace.log;
#	blktrace -d /dev/sdd -o - | blkparse -i - | tee ./log/${_tid}/${_tstamp}.${_tid}.blktrace.log;

	##
	## } case-specific code
	##
fi
