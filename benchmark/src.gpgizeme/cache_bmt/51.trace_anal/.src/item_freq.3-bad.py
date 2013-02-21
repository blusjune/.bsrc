#!/usr/bin/python



from collections import defaultdict

words = defaultdict(int)
input_word = True
while input_word:
	input_word = raw_input()
	if input_word:
		words[input_word] += 1

for word, freq in words.items():
	print word + " - " + str(freq)
