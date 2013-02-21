#!/bin/sh
##mk_ramdisk.sh
##tstamp:20121123_194022

read -p "ramdisk size in MB: " _msz_mb;
_msz_mb="${_msz_mb}M";
_rd="/tmp/ramdisk";

if [ ! -d $_rd ]; then
	mkdir $_rd;
fi

echo "sudo mount -t tmpfs -o size=$_msz_mb tmpfs $_rd";
sudo mount -t tmpfs -o size=$_msz_mb tmpfs $_rd;

df -h

echo "creating a big file to fill the ramdisk ...";
echo "dd if=/dev/random of=$_rd bs=1048576 count=$_msz_mb";
dd if=/dev/zero of=$_rd/bigfile bs=1048576 count=$_msz_mb;

df -h
