#!/usr/bin/python

#!/usr/bin/python3.2		## syntax error

lines = 0


import sys
from collections import defaultdict
from string import punctuation
from ctypes import *







class APtrn:

	def __init__(self):
		print "__init__ called"
		HITCNT, HITLOG = 0, 1
		self.addr = -1
		self.ptrn = [0, None]
		self.ptrn[HITLOG] = [None]

	def __call__(self, addr, atime):
		HITCNT, HITLOG = 0, 1
		self.addr = addr
		self.ptrn[HITCNT] = self.ptrn[HITCNT] + 1
		self.ptrn[HITLOG].append(atime)



# ap_list
ap_list = defaultdict(list)

ap_list[0].append(123220)
ap_list[1].append(123221)
ap_list[2].append(123222)
ap_list[3].append(123223)
ap_list[1].append(123224)
ap_list[1].append(123225)
ap_list[1].append(123226)

	
print ap_list

sys.exit(0)

# ap: access pattern
ap = [None, None, None]
ADDR, ACC_CNT, ACC_LOG = 0, 1, 2
ap[ADDR] = None
ap[ACC_CNT] = 0
ap[ACC_LOG] = [None]

ap_kv = defaultdict(list)






sys.exit(0)

ap = APtrn()
ap(1024, 74442)
ap(1025, 74444)
print ap.addr, ap.ptrn
sys.exit(0)







class AID_record(Structure):
	_fields_ = [
			("addr", c_int),
			("acc_cnt", c_int)
			]

rec1 = AID_record(1023, 1)
rec2 = AID_record(13, 1)
rec3 = AID_record(1343, 1)

print rec1.addr, rec1.acc_cnt
print rec2.addr, rec2.acc_cnt
print rec3.addr, rec3.acc_cnt

sys.exit(0)








words = defaultdict(int)

try:
	fn_i='infile'
	textf = open(fn_i, 'r')
except IOError:
	print 'CANNOT open file "%s" for reading' % fn_i
	sys.exit(0)

for line in textf:
	words[line] += 1
	lines += 1

for word, freq in words.items():
	print word.strip() + " : " + str(freq)
