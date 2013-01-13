#!/bin/sh

_log="_iozone.out";

date > $_log;
_harsh_storage_test=n export _harsh_storage_test;
bsc.bmt.iozone >> $_log;
date >> $_log;
