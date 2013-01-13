#!/bin/sh

## Wed Mar 14 21:59:17 KST 2012

_this_prog="$(basename $0)";
_iozone_executable="iozone.bver";
_iozone_cfg_file_default=".iozone.cfg";
_iozone_cfg_file="";




if [ $# -eq 0 ]; then
	_iozone_cfg_file="$_iozone_cfg_file_default";
elif [ $# -eq 2 ]; then
	if [ "X$1" = "X-f" -a "X$2" != "X" ]; then
		_iozone_cfg_file="$2";
	fi
else
	echo "[usage]";
	echo "";
	echo "$_this_prog";
	echo "	. run iozone with a configuration defined in './.iozone.cfg'";
	echo "	. if './.iozone.cfg' file does not exist,";
	echo "	parameters will be configured in an interactive manner";
	echo "";
	echo "$_this_prog -f <config_file>";
	echo "	. run iozone with a user specified <config_file>";
	echo "";
	echo "[.iozone.cfg example]";
	cat > .example${_iozone_cfg_file_default} << EOF


## .iozone.cfg

_raid_dev_id="md0";
_raid_level="raid5";

_out_fn="iozone_out.\$(date +%Y%m%d-%H%M%S)";
_test_fn_list="/mnt/\${_raid_dev_id}_\${_raid_level}/tf01 /mnt/\${_raid_dev_id}_\${_raid_level}/tf02 /mnt/\${_raid_dev_id}_\${_raid_level}/tf03 /mnt/\${_raid_dev_id}_\${_raid_level}/tf04";
_record_size="4096";
_testfile_size="409600";
_number_of_threads="4";

#_iozone_tests_to_run="-i 0 -i 1 -i 2 -i 8";
#_iozone_tests_to_run="-i 0";
#_iozone_tests_to_run="-i 1";
#_iozone_tests_to_run="-i 2";
_iozone_tests_to_run="-i 8";


EOF
	cat .example${_iozone_cfg_file_default};
	echo "";

	exit 0;
fi




if [ "X$_iozone_cfg_file" != "X" -a -f $_iozone_cfg_file ]; then
	. $_iozone_cfg_file;
else
	echo ">>> '$_iozone_cfg_file' file does not exist"
	echo ">>> Start interactive configuration <<<";

	if [ "X$_out_fn" = "X" ]; then
		read -p ">>> iozone output file name: " _out_fn; fi
	if [ "X$_test_fn_list" = "X" ]; then
		read -p ">>> list of files to be tested: " _test_fn_list; fi
	if [ "X$_record_size" = "X" ]; then
		read -p ">>> record size (#, #k, #m, #g): " _record_size; fi
	if [ "X$_testfile_size" = "X" ]; then
		read -p ">>> testfile size (#, #k, #m, #g): " _testfile_size; fi
	if [ "X$_number_of_threads" = "X" ]; then
		read -p ">>> number of threads: " _number_of_threads; fi
	if [ "X$_sleep_time_between_test" = "X" ]; then
		read -p ">>> sleep time between test: " _sleep_time_between_test; fi
	if [ "X$_iozone_tests_to_run" = "X" ]; then
		read -p ">>> iozone tests to run (-i # -i # -i # ...): " _iozone_tests_to_run; fi
fi




## options for general control : _opts_general
_show_bytes_xfered_by_each_child="-C";
_give_iops_results="-O";
_bind_processor_from_this="-P 1";
_create_offset_latency_files="-Q";
_generate_excel_report="-R";
_donot_unlink_tmp_files="-w";
_enable_cpu_stat_collection="-+u";

## options for harsh test (merciless to the storage) : _opts_harsh_test
_include_close_oper="-c";
_include_flush_oper="-e";
_use_direct_io="-I";
_write_synchronously="-o";
_purge_processor_cache="-p";

## options to determine the misc. color of the test : _opts_misc
_inject_random_access="-K";
_use_multiple_buffers="-m";




_opts_general="\
-b $_out_fn  \
$_show_bytes_xfered_by_each_child  \
$_give_iops_results \
-t $_number_of_threads  \
-F $_test_fn_list  \
$_bind_processor_from_this  \
$_create_offset_latency_files  \
$_generate_excel_report \
-r $_record_size  \
-s $_testfile_size  \
$_donot_unlink_tmp_files \
$_enable_cpu_stat_collection  \
-+F $_sleep_time_between_test \
$_iozone_tests_to_run";

if [ "X$_harsh_storage_test" = "Xy" ]; then
	_opts_harsh_test="\
$_include_close_oper  \
$_include_flush_oper  \
$_use_direct_io  \
$_write_synchronously  \
$_purge_processor_cache  \
";

else
	_opts_harsh_test=" ";
fi

_opts_misc=" ";




## run iozone, please!
echo ">>> $_iozone_executable $_opts_general $_opts_harsh_test $_opts_misc

";
$_iozone_executable $_opts_general $_opts_harsh_test $_opts_misc;




