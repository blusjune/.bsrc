#!/bin/sh




## trd: test result directory
## tsn: test serial number


_tstamp=$(tstamp);




## configuration variables
_start_delay=30;




if [ "X$1" != "X" ]; then
	_tname="$1";
else
	echo "Usage: $0 <test_name>";
	echo "	/* test name must be specified */";
	echo "	/* e.g., 'fs8gb' | 'fs1gb' | 'fs250mb' */";
	exit 0;
fi




## load cache-specific cache control commands (start/stop/status)
echo "## ----- cache mechanism to test -----";
(cd .cachectrl; ls -1 .cachectrl_cmdset.*) | sed -e 's/\.cachectrl_cmdset\.\(.*\)/## \1/g';
read -p ">>> select cache mechanism to test: " _cache_mechanism;
_cachectrl_cmdset_file=".cachectrl/.cachectrl_cmdset.$_cache_mechanism";
if [ ! -f $_cachectrl_cmdset_file ]; then
	echo "## $_cachectrl_cmdset_file does not exist";
	exit 1;
else
	. $_cachectrl_cmdset_file;
	echo "## $_cachectrl_cmdset_file is loaded";
fi
echo "## _cachectrl_start ($_cachectrl_start)";
echo "## _cachectrl_stop ($_cachectrl_stop)";
echo "## _cachectrl_status ($_cachectrl_status)";




_tsn_file=".tsn";
if [ ! -f $_tsn_file ]; then
	echo "0" > $_tsn_file;
fi
_tsn_val="$(cat $_tsn_file)";
_tsn="$(printf "%03d" $_tsn_val)";
_tsn_val="$(echo $_tsn_val + 1 | bc)";
echo $_tsn_val > $_tsn_file;

_trd_root="../10.test_log";
_trd_radix="tsn${_tsn}.${_tstamp}.${_tname}";
_trd="${_trd_root}/${_trd_radix}";
echo $_trd;

_test_lock="/tmp/.testlock.filebench";
#echo $_trd_radix > $_test_lock;
echo $_trd > $_test_lock;




mkdir -p $_trd;

####
####
####
#### test body begin {


(cd $_trd;

ln -s ../../30.filebench/..fb_exec_core.sh;
ln -s ../../30.filebench/..fb_testcase_mgmt.sh;
ln -s ../../30.filebench/.mnt;
ln -s ../../30.filebench/.workload;

_log="tlog.${_tstamp}.${_tname}";
touch $_log;




echo ">>> filebench test ($_tstamp:$_tname) will start in $_start_delay seconds";
sleep $_start_delay;




$_cachectrl_stop; sleep 1; # velobit stop; sleep 1;
$_cachectrl_status >> $_log; sleep 1; # velobit status >> $_log; sleep 1;
echo "test started (cache-off): $(tstamp)" >> $_log;
./..fb_testcase_mgmt.sh $_tname;
echo "test finished (cache-off): $(tstamp)" >> $_log;

echo "



----- ----- ----- -----



" >> $_log;

$_cachectrl_start; sleep 1; # velobit start; sleep 1;
$_cachectrl_status >> $_log; sleep 1; # velobit status >> $_log; sleep 1;
#(cd ../../49.cache_stat; _BDX)
echo "test started (cache-on): $(tstamp)" >> $_log;
./..fb_testcase_mgmt.sh $_tname;
echo "test finished (cache-on): $(tstamp)" >> $_log;
)


#### test body end }
####
####
####


rm $_test_lock;
exit 0;




