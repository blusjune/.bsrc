#!/bin/sh
##.bdx.0100.y.run_iotrace.sh
##_ver=20130107_200141
##_ver=20130218_133133
##_ver=20130219_104952


. ../20.conf/.tconf.sh


_tstamp="$(cat ../${_tmpdir}/${_tlog_wloadgen})";
_tdir="${_tdir_root}/${_tstamp}";
if [ -d $_tdir_root ]; then
	if [ ! -d $_tdir ]; then
		mkdir $_tdir;
	fi
else
	echo "#>> ERROR: '$_tdir_root' does not exist -- EXIT";
	exit 1;
fi


_bpob_file="bpob.$_tstamp"; # BPOB: BlkParse Output in a Binary format
_bpot_file="bpot.$_tstamp"; # BPOT: BlkParse Output in a Text format
echo "$_tstamp" > ../${_tmpdir}/${_tlog_iotrace};
#blktrace $_blktrace_opt_target_dev -o - | blkparse -i - -d $_tdir/$_bpob_file > $_tdir/$_bpot_file
blktrace $_blktrace_opt_target_dev -o - | blkparse -i - -d $_tdir/$_bpob_file




