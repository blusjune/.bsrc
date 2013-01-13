#!/bin/sh

##tstamp: 20121126_220733

_this_prog="freemem_stat";
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

	echo ">>> start $_this_prog ($_tid $_tstamp)";
	free -lt -s 1 -c 28800 > ./log/${_tid}/${_tstamp}.${_tid}.$_this_prog.log; # 28800: 8 hours

	##
	## } case-specific code
	##
fi




