#!/usr/bin/python

#!/usr/bin/python3.2		## syntax error

lines = 0


from collections import defaultdict
from string import punctuation

words = defaultdict(int)

try:
	fn_i='infile'
	textf = open(fn_i, 'r')
except IOError:
	print 'CANNOT open file "%s" for reading' % fn_i
	import sys
	sys.exit(0)

for line in textf:
	words[line] += 1
	lines += 1

for word, freq in words.items():
	print word.strip() + " : " + str(freq)
