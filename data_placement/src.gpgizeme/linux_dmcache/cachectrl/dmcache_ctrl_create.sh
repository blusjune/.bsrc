#!/bin/sh
## 20120827_014951


if [ "X$1" = "X" ]; then
	read -p "_dmcache_name: " _dmcache_name;
else
	_dmcache_name="$1";
fi


_01_srcdev_offset_start="0";
_02_srcdev_offset_end="131072";
_03_dmcache_target="cache"; # immutable
_04_target_media="/dev/sdc1";
_05_cache_media="/dev/sdb1";
_06_dmcache_param__flag_load_cache_metadata="0"; # '1' means "please reuse previous data (cache configuration and existing cached data) stored on the cache device"; '0' means new cache is started from the scratch;
_07_dmcache_param__cache_blocksize="8"; # unit: number of blocks ('8' means eight 512-byte blocks, i.e., 4096 bytes)
_08_dmcache_param__cache_capacity="65536"; # unit: number of blocks
_09_dmcache_param__cache_associativity="256";
_10_dmcache_param__writeback="1"; # if set, write back is enabled; otherwise, write back is disabled;


echo \
	$_01_srcdev_offset_start \
	$_02_srcdev_offset_end \
	$_03_dmcache_target \
	$_04_target_media \
	$_05_cache_media \
	$_06_dmcache_param__flag_load_cache_metadata \
	$_07_dmcache_param__cache_blocksize \
	$_08_dmcache_param__cache_capacity \
	$_09_dmcache_param__cache_associativity \
	$_10_dmcache_param__writeback \
	| dmsetup create $_dmcache_name;


# echo 0 131072 cache /dev/sdc1 /dev/sdb1 0 8 65536 256 1 | dmsetup create dm_cache
