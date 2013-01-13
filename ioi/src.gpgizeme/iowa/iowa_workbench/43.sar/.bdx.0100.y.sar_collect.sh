#!/bin/sh

_testlock=".testlock.iowa";

_trd="$(cat $_testlock)";
_trd_radix="$(basename $_trd)";
if [ "X$_trd_radix" = "X" ]; then
	echo ">>> TEST IS NOT READY YET. Exit this program (sar)";
	exit 0;
fi

_session_id="$_trd_radix" _trd="$_trd" bsc.sar.collect;
