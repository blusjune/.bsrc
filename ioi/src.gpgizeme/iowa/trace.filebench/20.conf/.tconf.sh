#!/bin/sh
##_ver=20130218_140415




_tmpdir="30.tmp";			# temporary directory
_tlog_wloadgen=".tlog.wloadgen";	# testlog for workload generation
_tlog_iotrace=".tlog.iotrace";		# testlog for iotrace
_tdir_root="tdir";			# test directory root

_filebench_wload_src="wloadsrc.f";	# workload file source
_filebench_wload_exe="wloadexe.f";	# workload file executable

_filebench_wload_src_target="videoserver.f";
#_filebench_wload_src_target="webserver.f";
#_filebench_wload_src_target="varmail.f";
#_filebench_wload_src_target="fileserver.f";
#_filebench_wload_src_target="mongo.f";
#_filebench_wload_src_target="netsfs.f";
#_filebench_wload_src_target="networkfs.f";
#_filebench_wload_src_target="oltp.f";
#_filebench_wload_src_target="tpcso.f";

_filebench_runntime_in_sec="3600";
#_filebench_runntime_in_sec="60";

_blktrace_opt_target_dev="-d /dev/sdd";




