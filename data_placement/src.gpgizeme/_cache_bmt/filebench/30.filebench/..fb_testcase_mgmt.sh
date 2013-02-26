#!/bin/sh
## ..fb_exec_wrapper.sh
## .bdx.0100.y.exec_filebench_and_tracing.sh

## 20120720_164021
## 20120727_081432
## 20120727_093610
## 20120728_003801




if [ "X$(id -u)" != "X0" ]; then
	echo ">>> You must have root privilege, exit this program";
	exit 0
fi
if [ "X$_tname" = "X" ]; then
	if [ "X$1" = "X" ]; then
		echo "Usage: $0 <test_name>";
		echo ">>> test name must be specified";
		exit 0;
	else
		_tname="$1";
	fi
fi




_test_running_time="600"; # 10 minutes




test_iu04_fio()
{
_test_func="iu04_fio.$(tstamp)"
_mntdir=".mnt/fioa1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x1_fs00" ./..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x2_fs00" ./..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x4_fs00" ./..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x8_fs00" ./..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x16_fs00" ./..fb_exec_core.sh;
}




test_iu05_fio()
{
_test_func="iu05_fio.$(tstamp)"
_mntdir=".mnt/fioa1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x1_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x2_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x4_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x8_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="fioa" _fb_run_time="$_test_running_time" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x16_fs00" ..fb_exec_core.sh;
}




test_iu06_ocz()
{
_test_func="iu06_ocz.$(tstamp)"
_mntdir=".mnt/sdb1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x1_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x2_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x4_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x8_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x16_fs00" ..fb_exec_core.sh;
}




test_iu07_samsung()
{
_test_func="iu07_sam.$(tstamp)"
_mntdir=".mnt/sdb1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x1_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x2_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x4_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x8_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x16_fs00" ..fb_exec_core.sh;
}




test_iu10_intel()
{
_test_func="iu10_int.$(tstamp)"
_mntdir=".mnt/sdb1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.intel_320_111GB.wl_par_rw_x1_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.intel_320_111GB.wl_par_rw_x2_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.intel_320_111GB.wl_par_rw_x4_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.intel_320_111GB.wl_par_rw_x8_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="sdb" _fb_run_time="$_test_running_time" _fb_workload="wl.intel_320_111GB.wl_par_rw_x16_fs00" ..fb_exec_core.sh;
}




test_b05_hdd()
{
_test_func="b05_hdd.$(tstamp)"
_mntdir=".mnt/sdc1";

#(cd $_mntdir; rm -fr tdat_*);
#_test_title="$_test_func" _tcase_id="x1" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.hdd_16GB.wl_par_rw_x1_fs00" ..fb_exec_core.sh;

#(cd $_mntdir; rm -fr tdat_*);
#_test_title="$_test_func" _tcase_id="x2" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.hdd_16GB.wl_par_rw_x2_fs00" ..fb_exec_core.sh;

#(cd $_mntdir; rm -fr tdat_*);
#_test_title="$_test_func" _tcase_id="x4" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.hdd_16GB.wl_par_rw_x4_fs00" ..fb_exec_core.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.hdd_16GB.wl_par_rw_x8_fs00" ..fb_exec_core.sh;

#(cd $_mntdir; rm -fr tdat_*);
#_test_title="$_test_func" _tcase_id="x16" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.hdd_16GB.wl_par_rw_x16_fs00" ..fb_exec_core.sh;
}




test_b05_hdd_tpcso()
{
_test_func="b05_hdd_tpcso.$(tstamp)"
_mntdir=".mnt/sdc1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.tpcso" ..fb_exec_core.sh;
}




test_b05_hdd_oltp()
{
_test_func="b05_hdd_oltp.$(tstamp)"
_mntdir=".mnt/sdc1";

#(cd $_mntdir; rm -fr tdat_*);
(cd $_mntdir; rm -fr tdat_* datafiles logfile);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="wl.oltp" ..fb_exec_core.sh;
}




test_ssdcache_randrw_r2w2()
{
_test_func="randrw.$(tstamp)"
_wl_name="wl.randrw_r2w2_${_tname}";	# _tname ::= "fs250mb" | "fs500mb" | "fs1gb" | "fs2gb" | "fs4gb" | "fs8gb"
_mntdir=".mnt/sdc1";

(cd $_mntdir; rm -fr tdat_* largefile*);
_test_title="$_test_func" _tcase_id="${_tname}" _target_dev="sdc" _fb_run_time="$_test_running_time" _fb_workload="${_wl_name}" ..fb_exec_core.sh;

}




#####
##### Execution!
#####

#test_iu04_fio
#test_iu05_fio
#test_iu06_ocz
#test_iu07_samsung
#test_iu10_intel
#test_b05_hdd
#test_b05_hdd_tpcso
#test_b05_hdd_oltp
test_ssdcache_randrw_r2w2




