#!/bin/sh

if [ ! -d flashcache ]; then
	echo "> git clone https://github.com/facebook/flashcache.git";
	git clone https://github.com/facebook/flashcache.git
else
	echo "> 'flashcache/' directory already exists // exit program";
	exit 0;
fi
