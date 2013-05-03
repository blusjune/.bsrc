#!/bin/sh
#.bdx.0100.y.convert_vcproj_to_makefile.sh
#_ver=20130503_170350

(cd iometer-2006_07_27.common-src/src;
../../vcproj2cmake-code/scripts/vcproj2cmake.rb Iometer.vcproj;
cmake -G 'Unix Makefiles' .;
echo ">>> edit CMakeList.txt to avoid ignorable errors -- NOW escape to a shell";
bash; 
cmake -G 'Unix Makefiles' .;
make -f Makefile-Linux.x86_64 all;)
