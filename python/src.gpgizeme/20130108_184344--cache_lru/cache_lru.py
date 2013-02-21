#!/usr/bin/python3.2


class LRU_Cache(object):

	def __init__(self, original_function, maxsize=1000):
		print("LRU_Cache:__init__")
		self.original_function = original_function
		self.maxsize = maxsize
		self.mapping = {}
		self.call_count = 0

		PREV, NEXT, KEY, VALUE = 0, 1, 2, 3
		self.head = [None, None, None, None]        # oldest
		self.tail = [self.head, None, None, None]   # newest
		self.head[NEXT] = self.tail

#	def __call__(self, *key):
	def __call__(self, key):
		print("")
#		print("LRU_Cache:__call__", "| keyt", key, "|", self.mapping)
#		print("LRU_Cache:__call__", "| key:", key)
#		print("LRU_Cache:__call__", "| *key:", *key)
		print("LRU_Cache:__call__", "| key:", key)
		self.call_count = self.call_count + 1
		PREV, NEXT, KEY, VALUE = 0, 1, 2, 3
		mapping, head, tail = self.mapping, self.head, self.tail
		sentinel = object()

#		link = mapping.get(*key, sentinel)
		link = self.mapping.get(key, sentinel)
		if link is sentinel:
			value = self.original_function(key)
			if len(self.mapping) >= self.maxsize:
				print("		", "| len(self.mapping) >= self.maxsize | ", len(self.mapping))
				oldest = self.head[NEXT]
				next_oldest = oldest[NEXT]
				self.head[NEXT] = next_oldest
				next_oldest[PREV] = self.head
				del self.mapping[oldest[KEY]]
			last = self.tail[PREV]
			link = [last, self.tail, key, value]
			self.mapping[key] = last[NEXT] = self.tail[PREV] = link
			print("	", "| link is sentinel // [key,value]:", key, value, " // link:", link[KEY], link[VALUE])
		else:
			link_prev, link_next, key, value = link
			link_prev[NEXT] = link_next
			link_next[PREV] = link_prev
			last = self.tail[PREV]
			last[NEXT] = self.tail[PREV] = link
			link[PREV] = last
			link[NEXT] = self.tail
			print("	", "|", "link is NOT sentinel // [key,value]:", key, value, "link:", link)
		return value

if __name__ == '__main__':
#	p = LRU_Cache(ord, maxsize=3)
#	for c in 'abcdecaeaa':
#	for c in a :
#		print(c, p(c))

	words = ['12', '34', '56', '78', '999', '12', '1818', '34']
	print(words);

	p = LRU_Cache(str, maxsize=3)
#	p = LRU_Cache(str)				# maxsize to 1000 by default

	for c in words :
		print(c, p(c))


