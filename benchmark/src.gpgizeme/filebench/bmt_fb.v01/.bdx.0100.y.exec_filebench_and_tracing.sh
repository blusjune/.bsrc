#!/bin/sh
## .bdx.0100.y.exec_filebench_and_tracing.sh

## 20120720_164021
## 20120727_081432
## 20120727_093610
## 20120728_003801




if [ "X$(id -u)" != "X0" ]; then
	echo ">>> You must have root privilege, exit this program";
	exit 0
fi




test_iu04_fio()
{
_test_func="iu04_fio.$(tstamp)"
_mntdir=".mnt/fioa1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x1_fs00" ./..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x2_fs00" ./..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x4_fs00" ./..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x8_fs00" ./..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x16_fs00" ./..run_fb.sh;
}




test_iu05_fio()
{
_test_func="iu05_fio.$(tstamp)"
_mntdir=".mnt/fioa1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x1_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x2_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x4_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x8_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="fioa" _fb_run_time="60" _fb_workload="wl.fio_iodrive_294GB.wl_par_rw_x16_fs00" ..run_fb.sh;
}




test_iu06_ocz()
{
_test_func="iu06_ocz.$(tstamp)"
_mntdir=".mnt/sdb1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x1_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x2_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x4_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x8_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.ocz_agility3_111GB.wl_par_rw_x16_fs00" ..run_fb.sh;
}




test_iu07_samsung()
{
_test_func="iu07_sam.$(tstamp)"
_mntdir=".mnt/sdb1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x1_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x2_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x4_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x8_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.samsung_830_118GB.wl_par_rw_x16_fs00" ..run_fb.sh;
}




test_iu10_intel()
{
_test_func="iu10_int.$(tstamp)"
_mntdir=".mnt/sdb1";

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x1" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.intel_320_111GB.wl_par_rw_x1_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x2" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.intel_320_111GB.wl_par_rw_x2_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x4" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.intel_320_111GB.wl_par_rw_x4_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x8" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.intel_320_111GB.wl_par_rw_x8_fs00" ..run_fb.sh;

(cd $_mntdir; rm -fr tdat_*);
_test_title="$_test_func" _tcase_id="x16" _target_dev="sdb" _fb_run_time="60" _fb_workload="wl.intel_320_111GB.wl_par_rw_x16_fs00" ..run_fb.sh;
}




#####
##### Execution!
#####

test_iu04_fio
#test_iu05_fio
#test_iu06_ocz
#test_iu07_samsung
#test_iu10_intel




