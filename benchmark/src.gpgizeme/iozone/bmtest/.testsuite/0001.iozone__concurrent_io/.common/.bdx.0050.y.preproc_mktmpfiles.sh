#!/bin/sh

## preprocessing: to make temporary files before starting main benchmark


. .iozone.cfg;


_timestamp="$(date +%Y%m%d_%H%M%S)";
_sleep_coef=1;

_log=".log.mktmpfiles.$_timestamp";
_1kb=1024;
_dd_bs="$(echo $_record_size * $_1kb | bc)";
_dd_count="$(echo $_testfile_size / $_record_size | bc)";

echo ">>> started at: $(date)" > $_log;
_t_1=$(date +%s);

_i=0;
while [ $_i -lt $_number_of_threads ];
do
	_i=$(expr $_i + 1);
	_tfname=$(printf "tf%02d" $_i);
	_dd_tgtfile="/mnt/${_dev_id}_${_dev_descr}/$_tfname";
	if [ -f $_dd_tgtfile ]; then
		echo "$_dd_tgtfile already exists -- no need to generate file";
	else
		echo ">>> dd bs=$_dd_bs count=$_dd_count if=/dev/zero of=/mnt/${_dev_id}_${_dev_descr}/$_tfname";
		dd bs=$_dd_bs count=$_dd_count if=/dev/zero of=/mnt/${_dev_id}_${_dev_descr}/$_tfname;
	fi
done

echo ">>> completed at: $(date)" >> $_log;
_t_2=$(date +%s);

_t_diff=$(echo "($_t_2 - $_t_1)" | bc);
_t_sleep=$(echo "$_sleep_coef * $_t_diff" | bc);
sleep $_t_sleep;
