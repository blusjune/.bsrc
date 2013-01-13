#!/bin/sh

_timestamp=$(tstamp);
_devlist="sdb sdc sdd";

for _i in $_devlist; do
	smartctl --all /dev/$_i > log.smart_info_before.$_i.$_timestamp
done

