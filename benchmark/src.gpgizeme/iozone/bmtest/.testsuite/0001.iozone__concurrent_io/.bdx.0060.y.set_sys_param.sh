#!/bin/sh


_devlist="sdb sdc sdd";

_disktype="ssd";
#_disktype="hdd";


##
##
##


## /sys/block/xxx/queue/ options

_nomerges_for_ssd="1"
_rotational_for_ssd="0"
_scheduler_for_ssd="noop"

_nomerges_for_hdd="0"
_rotational_for_hdd="1"
_scheduler_for_hdd="deadline"


## setting up device specific options

case $_disktype in 

"ssd"|"SSD")
	_nomerges_opt=$_nomerges_for_ssd;
	_rotational_opt=$_rotational_for_ssd;
	_scheduler_opt=$_scheduler_for_ssd;
	;;

"hdd"|"HDD")
	_nomerges_opt=$_nomerges_for_hdd;
	_rotational_opt=$_rotational_for_hdd;
	_scheduler_opt=$_scheduler_for_hdd;
	;;

esac


## setting up the /sys/block/*/queue/ parameters

for _dev in $_devlist;
do
	(
	cd /sys/block/$_dev/queue;
	echo $_nomerges_opt > nomerges;
	echo $_rotational_opt > rotational;
	echo $_scheduler_opt > scheduler;

	echo ">>> parameter setup result for $_dev:";
	echo "$_dev: nomerges: '$(cat nomerges)'";
	echo "$_dev: rotational: '$(cat rotational)'";
	echo "$_dev: scheduler: '$(cat scheduler)'";
	)
done

echo "sleep for a moment (10 sec) to give you a parameter check time";
sleep 10;




