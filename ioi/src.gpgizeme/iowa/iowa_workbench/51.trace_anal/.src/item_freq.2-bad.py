#!/usr/bin/python

itemsFreq = {}

while True:
	inputItem = input()
	if (inputItem != ""):
		itemsFreq[inputItem] = itemsFreq.get(inputItem, 0) + 1
	else:
		break

for item, freq in itemsFreq.items():
	print item + " - " + str(freq)

