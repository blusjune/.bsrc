#!/bin/sh
## 20121016_012334




chk_prog()
{
	if [ "X$(which $_target_prog)" = "X" ]; then
		echo ">>> ERROR: $_target_prog does not exist (now exit this program)";
		exit 1;
	else
		echo ">>> OK: $_target_prog exists";
	fi
}

chk_privilege()
{
	if [ "X$(id -u)" = "X0" ]; then
		echo ">>> OK: you have root privilege";
	else
		echo ">>> ERROR: you must have root privilege (now exit this program)";
		exit 1;
	fi
}




## sanity check
chk_privilege;
_target_prog="filebench" chk_prog;
_target_prog="blktrace" chk_prog;
_target_prog="sar" chk_prog;
_target_prog="iostat" chk_prog;
#_target_prog="velobit" chk_prog;
#_target_prog="velomgmt" chk_prog;




echo "INFO: Please make sure that
'\$dir' (in the file '.workload/wl.randrw_r2w2_fs*') corresponds to
'\$_mntdir' (in the file '..fb_testcase_mgmt.sh')";
bsc.countdown 10;




./.run_test.sh "fs250mb"
#./.run_test.sh "fs500mb"
#./.run_test.sh "fs1gb"
#./.run_test.sh "fs2gb"
#./.run_test.sh "fs4gb"
#./.run_test.sh "fs8gb"

## _wl_name="wl.randomrw_r2w2_${_tname}";
## _tname ::= "fs250mb" | "fs500mb" | "fs1gb" | "fs2gb" | "fs4gb" | "fs8gb"

