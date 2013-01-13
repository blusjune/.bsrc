#!/bin/sh
## uFLIP test environment setup
## Thu Jun 28 23:41:49 KST 2012




echo "!!! may be obsoleted !!! (exit 0)";
exit 0;




_timestamp=$(date +%Y%m%d_%H%M);
_flashio_src_dir="flashio-2.2.99.2";

echo ">>> current directory is $(pwd)";
read -p ">>> benchmark test name ? (e.g., 'raid5_intel' OR 'ssd_intel'): " _tname;
_tdir="bmt_${_timestamp}-${_tname}";


if [ ! -d .b_src ]; then
        echo ">>> cloning .b_src from github.com ...";
        git clone git://github.com/blusjune/.b_src.git;
fi
if [ ! -d work ]; then
        echo ">>> creating 'work' directory here ...";
	mkdir work;
fi

ln -s .b_src/u/uflip/$_flashio_src_dir src;
cp -r src/data/ work/$_tdir;
ln -s work/$_tdir .this;

echo ">>> Great! ENJOY Test!!!"; 
