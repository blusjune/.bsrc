#!/bin/sh
## 20120827_014951


if [ "X$1" = "X" ]; then
	read -p "_dmcache_name: " _dmcache_name;
else
	_dmcache_name="$1";
fi


#umount /dev/mapper/$_dmcache_name;
dmsetup remove $_dmcache_name;
