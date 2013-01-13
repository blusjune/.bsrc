#!/bin/sh

_ts="$(tstamp)";
read -p "dm disk name? " _disk_name;
_new_dm_disk="/dev/mapper/$_disk_name";




_mk_dmtab_2_linearspan()
{
cat > dmtab << EOF_2
0 1024 linear /dev/sdb1 0
1024 1024 linear /dev/sdc1 0
EOF_2
}




_mk_dmtab_3_striping()
{
cat > dmtab << EOF_3
0 2048 striped 2 32 /dev/sdb1 0 /dev/sdc1 0
EOF_3
}




_mk_dmtab_2_linearspan;




dmsetup create $_disk_name < dmtab
dmsetup ls

# set readahead to 0, check block device size
blockdev --setra 0 $_new_dm_disk;
blockdev --getsz $_new_dm_disk;





