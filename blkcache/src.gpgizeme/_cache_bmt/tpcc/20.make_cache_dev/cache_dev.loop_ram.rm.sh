#!/bin/sh


_fs_point="/mnt/ramdisk/0";
_loopdev="/dev/loop0";
_cache_dev_img="$_fs_point/cache_dev.img";

losetup -d $_loopdev;
umount $_fs_point;

echo ">>>      ----- rm.RESULT -----      <<<";
mount | grep $_fs_point;
losetup -a;
