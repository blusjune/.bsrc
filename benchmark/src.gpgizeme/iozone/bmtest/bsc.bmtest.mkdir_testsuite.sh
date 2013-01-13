#!/bin/sh

## bsc.bmtest.testsuite_mkdir.sh
##
## Fri Mar 16 00:51:31 KST 2012
## Mon Mar 19 19:30:16 KST 2012


_prog_name="$(basename $0)";
_timestamp="$(date +%Y%m%d-%H%M)";
_dir="bmtest.$_timestamp";
_iozone_cfg=".iozone.cfg";


if [ "X$1" = "X-h" -o "X$1" = "X--help" ]; then
	echo "$_prog_name <testsuite>";
	echo "	: create a base directory for testsuite specified";
	echo "";
	echo "testsuite supported:";
	echo "	0: reserved";
	echo "	1: iozone -- concurrent I/O effect test";
	exit 0;
fi


if [ "X$_ts" = "X" ]; then
	read -p ">>> testsuite: " _ts;
fi
case $_ts in
"1")
	_ts_descr="iozone__concurrent_io";
	;;
*)
	echo ">>> undefined testsuite identifier";
	exit 1;
esac


_tsdir="$(printf "%04d.%s" $_ts $_ts_descr)";
echo ">>> testsuite directory $_tsdir will be created";


mkdir $_dir;
(
	cd $_dir;
	bsc.storage.mktooldir;
	(cd ${HOME}/.b/.src/x/bmtest/.testsuite/; tar cf - $_tsdir) | tar xf -;
	cd $_tsdir;
	read -p ">>> check and edit configuration files? [y|n] " _ans_1;
	if [ "X$_ans_1" = "Xy" -o "X$_ans_1" = "XY" ]; then
		. .bdx.0050.n.edit_conf_files.sh;
	fi
);


