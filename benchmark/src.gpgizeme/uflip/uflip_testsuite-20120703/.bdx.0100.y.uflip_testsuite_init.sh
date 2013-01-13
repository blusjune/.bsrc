#!/bin/sh
## do this!

_timestamp=$(date +%Y%m%d_%H%M);
_sudo="$(which sudo)";

read -p "test description (e.g., 'raid0_intel' or 'single_ocz') : "  _tdescr;
_tdir="bmt_uflip_${_timestamp}-$_tdescr";

if [ ! -d work ]; then
	mkdir work;
fi


( cd work;

bsc.storage.mktooldir;
ln -s .storage_mgmt/raid_w_ssd/raid .raid_conf;

cp -r ../../.uflip_src_root/ .;
(cd .uflip_src_root; ./configure; make clean; make; $_sudo make install);
cp -r .uflip_src_root/data/ $_tdir;

)


if [ -h _THIS ]; then
	rm _THIS;
fi
ln -s work/$_tdir _THIS;


