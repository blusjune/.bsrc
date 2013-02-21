#!/usr/bin/python
##-------------------------------------------
import os;
_infile = os.environ['_INFILE'];
f = open(_infile, 'r');
_addr_prev = 0;
_seek_dist = 0;
for line in f:
#	_addr_curr = int(line.split()[10]);
	_addr_curr = int(line.split()[0]);
	_seek_dist = _addr_curr - _addr_prev;
	_addr_prev = _addr_curr;
	print _seek_dist;
f.close()
##-------------------------------------------
