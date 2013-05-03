#!/bin/sh
#.bdx.0100.y.uflip_testsuite_init.sh
#_ver=20130503_103914


_timestamp=$(date +%Y%m%d_%H%M);
_sudo="$(which sudo)";


read -p "UFLIP> test description (e.g., 'raid0_intel' or 'single_ocz') : "  _tdescr;
_tdir="_BMT_UFLIP.${_timestamp}-$_tdescr";
_build_dir=".build.d";
_test_dir="test.d";
if [ ! -d "$_tdir" ]; then
	mkdir -p "$_tdir";
fi


( cd "$_tdir";
cp -pr ../.dirs/.uflip_src_root/ $_build_dir;
(cd $_build_dir; ./configure; make clean; make; $_sudo make install);
cp -pr $_build_dir/data $_test_dir;
cp -pr $_build_dir/src/FlashIO $_test_dir;
)


echo "

---
>>> OK! move to '$_tdir/$_test_dir' directory and execute '_BDX'
";


