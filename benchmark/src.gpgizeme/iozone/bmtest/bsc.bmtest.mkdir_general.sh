#!/bin/sh

## bsc.bmtest.mkdir.sh
##
## Fri Mar 16 00:51:31 KST 2012
## Mon Mar 19 19:30:16 KST 2012


_timestamp="$(date +%Y%m%d-%H%M)";
_dir="bmtest.$_timestamp";


mkdir $_dir;
(
	cd $_dir;
	bsc.storage.mktooldir;
	(cd ${HOME}/.b/.src/x/bmtest; tar cf - .skel) | tar xf -;
	cp -r .skel t.0001;
);


