#!/bin/sh
## bsc.hadoop.hdfs_io_test.test_case_generator.sh
##
## Thu Apr 12 18:30:38 KST 2012

if [ "$#" != "2" ]; then
	echo "Usage: $(basename $0) <number_of_workload_superposition> <file_size_in_MB>"
	exit 1;
else
	_num_of_workload_superposition=$1;
	_file_size_in_MB=$2;
fi

_timestamp=$(date +%Y%m%d_%H%M%S);
_hostname=$(hostname);
_tid="taskid-${_timestamp}-${_hostname}--${_num_of_workload_superposition}x,${_file_size_in_MB}MB" # task file
_ttlw="/home/hduser/ttlw"
_ttlw_tid="$_ttlw/$_tid"
_script_main=".bdx.main.$_tid.sh"
_script_sub=".bdx.sub.$_tid"
_sf="file_1MB" # source file

cd ~hduser;
if [ ! -d ttlw ]; then
	echo "$_ttlw does not exist"
	if [ ! -d /x/ttl/work ]; then
		echo "please create /x/ttl/work and then retry!"
		exit 2;
	fi
	echo "create symbolic link $_ttlw"
	ln -s /x/ttl/work ttlw;
fi

cd ttlw;
if [ ! -d ${_ttlw_tid} ]; then
	echo now create directory: ${_ttlw_tid}
	mkdir -p ${_ttlw_tid};
fi
mkdir $_ttlw_tid/r $_ttlw_tid/w $_ttlw_tid/logs;
dd if=/dev/zero of="$_ttlw_tid/r/$_sf" bs=1048576 count=$_file_size_in_MB

## create source file
cd $_ttlw_tid;
echo "## $_tid" > $_script_main
#echo "hadoop dfs -mkdir /t ;" >> $_script_main
echo "hadoop dfs -mkdir /t/$_tid ;" >> $_script_main
echo "hadoop dfs -mkdir /t/$_tid/r ;" >> $_script_main
echo "hadoop dfs -mkdir /t/$_tid/w ;" >> $_script_main
echo "hadoop dfs -copyFromLocal $_ttlw_tid/r/$_sf /t/$_tid/r ;" >> $_script_main

_i=1;
_j=2;
while [ $_i -lt $_num_of_workload_superposition ]; do
	printf ". $_script_sub.$_i > $_ttlw_tid/logs/log-%02d 2>&1 & \\" $_i >> $_script_main
	echo >> $_script_main
	printf ". $_script_sub.$_j > $_ttlw_tid/logs/log-%02d 2>&1 & \\" $_j >> $_script_main
	echo >> $_script_main
	_i=$(expr $_i + 2);
	_j=$(expr $_j + 2);
done
echo ":" >> $_script_main

_i=1;
_j=2;
while [ $_i -lt $_num_of_workload_superposition ]; do
	printf "time hadoop dfs -copyFromLocal $_ttlw_tid/r/$_sf /t/$_tid/w/file-%02d" $_i > $_script_sub.$_i
	printf "time hadoop dfs -copyToLocal /t/$_tid/r/$_sf $_ttlw_tid/w/file-%02d" $_j > $_script_sub.$_j
	chmod 755 $_script_sub.$_i $_script_sub.$_j
	_i=$(expr $_i + 2);
	_j=$(expr $_j + 2);
done

chmod 755 $_script_main

cd /tmp; rm btask > /dev/null 2>&1; ln -s $_ttlw_tid btask
echo "Good! please go to the /tmp/btask directory, and just do it!"

