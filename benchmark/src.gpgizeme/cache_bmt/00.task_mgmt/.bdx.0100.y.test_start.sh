#!/bin/sh

##tstamp: 20121116_134213
##

_tstamp=$(tstamp);
_testlock="/tmp/.testlock.iowa";

if [ "X$(id -u)" != "X0" ]; then
	echo "ERROR: you should have root privilege (exit now)";
	exit 1;
fi


if [ "X$_tid" = "X" ]; then
	read -p "Test ID (tid): " _tid;
fi
echo ">>> $_tid#$_tstamp";
echo "$_tid#$_tstamp" > $_testlock;




_run_blktrace()
{
	(cd ../41.blktrace; _BDX;)
}

_run_freemem_stat()
{
	(cd ../44.freemem_stat; _BDX;)
}




##
_run_blktrace &
_run_freemem_stat &
