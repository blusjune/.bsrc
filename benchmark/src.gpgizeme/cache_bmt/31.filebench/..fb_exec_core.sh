#!/bin/sh
## .run_fb.sh

## 20120720_164021
## 20120727_081432
## 20120728_010802




if [ "X$(id -u)" != "X0" ]; then
	echo ">>> You must have root privilege, exit this program";
	exit 0
fi
_tstamp="$(tstamp)";




if [ "X$_test_title" = "X" ]; then
	echo ">>> ERROR: _test_title is not defined"; exit 1;
fi
if [ "X$_tcase_id" = "X" ]; then
	echo ">>> ERROR: _tcase_id is not defined"; exit 2;
fi
if [ "X$_target_dev" = "X" ]; then
	echo ">>> ERROR: _target_dev is not defined"; exit 3;
fi
if [ "X$_fb_workload" = "X" ]; then
	_fb_workload="workload.f";
	if [ ! -f $_fb_workload ]; then
		echo ">>> ERROR: '$_fb_workload' does not exist"; exit 4;
	fi
fi
if [ "X$_fb_run_time" = "X" ]; then
	echo ">>> ERROR: _fb_run_time is not defined"; exit 5;
fi




_tdir="td_${_test_title}/${_tcase_id}--${_fb_workload}";
_log_prefix="log.${_tcase_id}.${_fb_workload}";
_exec_prefix="exec.${_tcase_id}.${_fb_workload}";
_biotr_log="$_log_prefix.biotr.txt";
_iostat_log="$_log_prefix.iostat.txt";
_fb_log="$_log_prefix.filebench.txt";
_run_trace="$_exec_prefix.run_trace.sh";
_result_n_io_opr="n_io_opr.$_tcase_id";		# result data: number of I/O operations




if [ ! -d $_tdir ]; then
	mkdir -p $_tdir;
fi




(cd $_tdir;
echo ">>> directory is changed to $_tdir";
echo ">>> pwd: $(pwd)";

_fb_workload_part_1="../../.workload/${_fb_workload}";
_fb_workload_part_2=".tmp.${_fb_workload}.p2";
_fb_workload_file="${_fb_workload}.f";
echo "
run $_fb_run_time" > $_fb_workload_part_2
cat $_fb_workload_part_1 $_fb_workload_part_2 > $_fb_workload_file

_iostat_run_count="$_fb_run_time";
_target_dev_path="/dev/$_target_dev";
#_target_dev_path="/dev/fioa1";	# WARNING: this does not work!

cat > $_run_trace << EOF
(iostat -xkt 1 $_iostat_run_count > $_iostat_log) &
bsc.bio.trace $_target_dev_path > $_biotr_log;
#/usr/local/home/b/.b/x/sys/sh/bsc.bio.trace $_target_dev_path > $_biotr_log;
echo ">>> now iostat is running in background";
echo ">>> now btrace (blktrace + blkparse) is running in foreground";
echo ">>> just run 'bsc.bio.analysis $_biotr_log' later for detailed analysis";
jobs -l
EOF
chmod 755 $_run_trace;

echo ">>> ### start: filebench -f $_fb_workload_file";

filebench -f $_fb_workload_file | tee $_fb_log;

echo ">>> ### end: filebench -f $_fb_workload_file";
echo ">>> please terminate the sub-jobs (iostat and trace) gracefully"


cat $_fb_log | grep ops | grep rand-write | awk '{ print $2 }' | sed -e 's/ops//' > $_result_n_io_opr
cp $_result_n_io_opr ..
)




