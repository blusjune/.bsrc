#!/bin/sh

_ts="$(tstamp)";

if [ "X$1" = "X-h" -o "X$1" = "X--help" ]; then
	echo "[[ dmsetup guide ]]";
	echo "";
	echo ">>> WARNING: Please make sure that /dev/loop0 is in unmounted state"
	echo ">>> WARNING: Unfortunately if /dev/loop0 is mounted, then you'll get the following error"
	echo "";
	echo "Examples";
	echo "
	<message sprayed to stdout>
		device-mapper: reload ioctl failed: Invalid argument
		Command failed

	<dmesg informaton>
		[11015.872399] device-mapper: table: 252:0: linear: dm-linear: Device lookup failed
		[11015.872401] device-mapper: ioctl: error adding target to table
	";

	exit 0;
fi




read -p "Type the name of disk to create by DM: " _disk_name;
read -p "Type the name of loop device: " _loopdev_name;




_bad_disk="/dev/mapper/$_disk_name";

# create 100MB disk with loop device
dd if=/dev/zero of=fsimg_bs1024_count100000_${_ts}.img bs=1024 count=100000
losetup /dev/$_loopdev_name fsimg_bs1024_count100000_${_ts}.img
#mkfs -L LOOPFS_0810_0026 -t ext4 -b 1024 -c /dev/loop0 100000
#mount -t ext4 -o noatime /dev/loop0 /mnt/loop/d0









_mk_dmtab_1_baddisk()
{
# create new disk and map 9th sector to error target
# to simulate disk fail - 100MB disk with a bad 9th sector
## 0 8 linear /dev/loop0 0
## 8 1 error
## 9 991 linear /dev/loop0 9
cat > dmtab << EOF_1
0 100 linear /dev/$_loopdev_name 0
100 1 error
101 199899 linear /dev/$_loopdev_name 9
EOF_1
}


_mk_dmtab_2_linearspan()
{
cat > dmtab << EOF_2
0 1024 linear /dev/sdb1
1024 1024 /dev/sdc1
EOF_2
}


_mk_dmtab_3_striping()
{
cat > dmtab << EOF_3
0 2048 striped 2 32 /dev/sdb1 0 /dev/sdc1 0
EOF_3
}




_mk_dmtab_2_linearspan;




echo ">>> dmsetup create $_disk_name < dmtab";
dmsetup create $_disk_name < dmtab
echo ">>> dmsetup ls";
dmsetup ls

# set readahead to 0, check block device size
echo "blockdev --setra 0 $_bad_disk";
echo "blockdev --getsz $_bad_disk";
blockdev --setra 0 $_bad_disk;
blockdev --getsz $_bad_disk;

# dd should fail on the 51st sector
echo "
dd if=$_bad_disk of=/dev/null bs=512 count=99";
dd if=$_bad_disk of=/dev/null bs=512 count=99;
echo "
dd if=$_bad_disk of=/dev/null bs=512 count=100";
dd if=$_bad_disk of=/dev/null bs=512 count=100;
echo "
dd if=$_bad_disk of=/dev/null bs=512 count=101";
dd if=$_bad_disk of=/dev/null bs=512 count=101;




read -p "Do you want to clean-up the test environment? [y|n] " _ans;

if [ "X$_ans" = "Xy" ]; then
	echo "Cleaning-up is going to run:
	dmsetup ls;
	dmsetup remove $_disk_name;
	losetup -a;
	losetup -d $_loopdev_name;
	";

	dmsetup ls;
	dmsetup remove $_disk_name;
	losetup -a;
	losetup -d /dev/$_loopdev_name;
else
	echo "NOT cleaned up";
fi

