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
read -p "type the name of directory you just have created: " _bmtdir;
read -p "type the device name of flash device (e.g., '/dev/sdb1'): " _dev;

(
cd $_bmtdir;
_cmd_gen_prepare="./FlashIO GenPrepare Dev $_dev IOS 64 IOC 10000 IOC2 50000";
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
	echo ">>> Exit this program";
	exit 0;
fi




echo "-----";
echo ">>> Step 2. 'Bench.sh'";
read -p "Number of Runs [3]: " _runs;
read -p "IO request size in (512-byte) sectors [64]: " _ios;
read -p "Pause time in microseconds [1000]: " _pause;
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




