#!/bin/sh


_fs_point="/mnt/ramdisk/0";
_loopdev="/dev/loop0";
_cache_dev_img="$_fs_point/cache_dev.img";

mount -t tmpfs -o size=1100m tmpfs $_fs_point;
mount;
df -h;
echo "

";

dd if=/dev/zero of=$_cache_dev_img bs=1K count=1M;
losetup $_loopdev $_cache_dev_img;

echo ">>>      ----- mk.RESULT -----      <<<";
mount | grep $_fs_point;
losetup -a;

