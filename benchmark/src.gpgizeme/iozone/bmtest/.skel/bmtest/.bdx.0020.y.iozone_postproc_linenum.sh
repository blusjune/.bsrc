#!/bin/sh




#_flist=$(ls -1 Child_*.dat);
_flist=$(ls -1 *.dat);

_c=0;
for _i in $_flist; do
	_c=$(expr $_c + 1);
	_fn=$(printf "num.%04d.$_i.txt" $_c);
	more +2 $_i | cat -n > $_fn;
done
echo ">>> $_c files are post-processed";




