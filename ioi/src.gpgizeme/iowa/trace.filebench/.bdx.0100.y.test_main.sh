#!/bin/sh
##_ver=20130219_104009

if [ "X$(id -u)" != "X0" ]; then
	echo "#>> ERROR: you should have root priviledge -- EXIT";
	exit 1;
fi

echo "#>> cleaning up '30.tmp/' directory";
(cd 30.tmp; rm .tlog.*;)

echo "#>> ready for generating workload specified in '20.conf/.tconf.sh'";
echo "#>> please run iotrace (blktrace) in '34.iotrace/'";
(cd 32.wloadgen; _BDX;)


