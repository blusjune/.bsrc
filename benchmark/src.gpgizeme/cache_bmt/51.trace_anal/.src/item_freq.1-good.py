#!/usr/bin/python

from collections import defaultdict

items = "apple banana apple strawberry banana lemon appleberry"

d = defaultdict(int)
for item in items.split():
	d[item] += 1

print d
