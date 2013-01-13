#!/bin/sh

#cat /var/log/syslog > /mnt/sdc1/t/test.out

_tstamp=$(tstamp);
_ifile=/tmp/bigfile;
_ofile=/mnt/sdc1/t/test-$_tstamp.out
_bsize=4096
_count=10000

echo "use rand(1) command, please";
exit 0;

dd if=$_ifile of=$_ofile bs=$_bsize count=$_count;
