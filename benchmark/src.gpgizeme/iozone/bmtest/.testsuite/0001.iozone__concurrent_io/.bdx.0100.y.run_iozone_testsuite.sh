#!/bin/sh

## main execution driver to run all the tests

_timestamp="$(date +%Y%m%d-%H%M%S)";
_logfile="_testlog-$_timestamp";
_sleep_coef=1;

_tlist=$(ls -1d td.*);
echo ">>> target directories:
$_tlist
";

for _i in $_tlist; do
	echo "
	>>> executing $_i case
	" >> $_logfile;

	date >> $_logfile;

	_t_1=$(date +%s);
	( cd $_i/bmt; _BDX; )
	_t_2=$(date +%s);

	date >> $_logfile;
	df -k >> $_logfile;

	_t_diff=$(echo "($_t_2 - $_t_1)" | bc);
	_t_sleep=$(echo "$_sleep_coef * $_t_diff" | bc);
	echo "elapsed time: $_t_diff" >> $_logfile;
	echo "sleep $_t_sleep" >> $_logfile;
	sleep $_t_sleep;
done
