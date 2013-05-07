#!/bin/sh
## 20120607_142713 ## 'number of runs' to '3' (from 10)
## 20120607_143438 ## 'pause time' to '1000' (from 5000)

if [ "X$(id -u)" != "X0" ]; then
	echo ">>> ERROR: You SHOULD have root privilege to execute uFLIP properly!!!";
	echo ">>> Exit this program";
	exit 1;
fi

if [ ! -f FlashIO ]; then
	echo ">>> ERROR: 'FlashIO' program is not found here";
	echo ">>> Please build and locate 'FlashIO' here before going any further";
	echo ">>> Exit this program";
	exit 2;
fi

chmod 755 *.py
echo ">>> Now start 'setup.py' script ...";
./setup.py




_timestamp="$(date +%Y%m%d_%H%M%S)";
_execlog="log.uflip_exec.$_timestamp";
touch $_execlog;




echo "-----";
echo ">>> Step 1. 'Prepare.sh'";

_bmtdir_default="$(cat /tmp/.uflip.bmt_path)"; # data/setup.py wrote this information
read -p "type the name of directory you just have created [$_bmtdir_default]: " _bmtdir;
if [ "X$_bmtdir" = "X" ]; then
	_bmtdir="$_bmtdir_default";
fi

_dev_default="$(df -h $_bmtdir | grep '/dev/sd' | awk '{print $1}')";
read -p "type the device name of flash device [$_dev_default]: " _dev;
if [ "X$_dev" = "X" ]; then
	_dev="$_dev_default";
fi

_ios_default="8";
read -p "IO request size in (512-byte) sectors [$_ios_default]: " _ios;
if [ "X$_ios" = "X" ]; then
	_ios="$_ios_default";
fi




(
cd $_bmtdir;
_cmd_gen_prepare="./FlashIO GenPrepare Dev $_dev IOS $_ios IOC 10000 IOC2 50000";
echo $_cmd_gen_prepare;
$_cmd_gen_prepare;

if [ ! -f "Prepare.sh" ]; then
	echo ">>> ERROR: 'Prepare.sh' file does not exist";
	echo ">>> Exit this program";
	exit 3;
fi

chmod 755 Prepare.sh

echo "";
read -p "Do you want to edit 'Prepare.sh'? [y|n] " _ans;
if [ "X$_ans" = "Xy" -o "X$_ans" = "XY" ]; then
	vi Prepare.sh
fi

echo "";
read -p "Are you ready to run 'Prepare.sh'? [y|n] " _ans;
if [ "X$_ans" = "Xy" -o "X$_ans" = "XY" ]; then
	echo ">>> Now execute ./Prepare.sh ..."
	echo "uFLIP prepare start time: $(date +%Y%m%d_%H%M%S)" >> $_execlog;
	./Prepare.sh;
	echo "uFLIP prepare finish time: $(date +%Y%m%d_%H%M%S)" >> $_execlog;
else
	echo ">>> You chose not to run 'Prepare.sh' this time";
	read -p "Do you want to create and run 'Bench.sh' without executing 'Prepare.sh'? [y|n] " _ans;
	if [ "X$_ans" != "Xy" ]; then
		echo ">>> Exit this program";
		exit 0;
	fi
fi




echo "-----";
echo ">>> Step 2. 'Bench.sh'";
_runs_default="3";
read -p "Number of Runs [$_runs_default]: " _runs;
if [ "X$_runs" = "X" ]; then
	_runs=$_runs_default;
fi
_pause_default="1000";
read -p "Pause time in microseconds [$_pause_default]: " _pause;
if [ "X$_pause" = "X" ]; then
	_pause=$_pause_default;
fi
_cmd_gen_bench="./FlashIO GenBench Dev $_dev NbRun $_runs IOS $_ios Pause $_pause";
echo $_cmd_gen_bench;
$_cmd_gen_bench;

if [ ! -f "Bench.sh" ]; then
	echo ">>> ERROR: 'Bench.sh' file does not exist";
	echo ">>> Exit this program";
	exit 3;
fi

chmod 755 Bench.sh

echo "";
read -p "Do you want to edit 'Bench.sh'? [y|n] " _ans;
if [ "X$_ans" = "Xy" -o "X$_ans" = "XY" ]; then
	vi Bench.sh;
fi

echo "";
read -p "Are you ready to run 'Bench.sh'? [y|n] " _ans;
if [ "X$_ans" = "Xy" -o "X$_ans" = "XY" ]; then
	echo ">>> Now execute ./Bench.sh ..."
	echo "uFLIP bench start time: $(date +%Y%m%d_%H%M%S)" >> $_execlog;
	./Bench.sh;
	echo "uFLIP bench finish time: $(date +%Y%m%d_%H%M%S)" >> $_execlog;
else
	echo ">>> You chose not to run 'Bench.sh' this time";
	echo ">>> Exit this program";
	exit 0;
fi


( cd RESULTS; cat RES_*.csv > ALL_RESULTS.csv; ./process.py; )


)




