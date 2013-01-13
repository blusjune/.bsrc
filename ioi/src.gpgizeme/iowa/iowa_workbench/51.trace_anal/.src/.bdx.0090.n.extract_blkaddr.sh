#!/bin/sh

if [ "X$1" = "X" ]; then
	read "input file: " _if;
else
	_if=$1;
fi
echo "input file is set to $_if";

_of_1="$_if.1.out";
_of_2="$_if.2.A.out"; ## All (read + write)
_of_3="$_if.3.R.out"; ## Read
_of_4="$_if.4.W.out"; ## Write

cat $_if | awk '{ print "_" $7, "__" $6, "___" $1, $0 }' | grep -e '_[RW]' | grep '__Q' > $_of_1;
cat $_of_1 | awk '{ print $11 }' > $_of_2;
cat $_of_1 | grep -e '_R' | awk '{ print $11 }' > $_of_3;
cat $_of_1 | grep -e '_W' | awk '{ print $11 }' > $_of_4;


