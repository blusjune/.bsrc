#!/bin/sh

_log="_git-clone.log";
echo "linux-bcache.git: start at $(tstamp)" >> $_log
git clone http://evilpiepirate.org/git/linux-bcache.git
echo "linux-bcache.git: finished at $(tstamp)" >> $_log
echo "bcache-tools.git: start at $(tstamp)" >> $_log
git clone http://evilpiepirate.org/git/bcache-tools.git
echo "bcache-tools.git: finished at $(tstamp)" >> $_log
