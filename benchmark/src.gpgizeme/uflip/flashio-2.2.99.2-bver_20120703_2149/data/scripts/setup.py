#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This file is part of the uFLIP software. See www.uflip.org
# 
# uFLIP is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# uFLIP is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with uFLIP.  If not, see <http://www.gnu.org/licenses/>.
# 
# uFLIP was initially developed based on SQLIO2 by Leonard Chung although
# almost all SQLIO2 code have disappeared. (leonard@ssl.berkeley.edu - see
# http://research.microsoft.com/en-us/um/siliconvalley/projects/sequentialio/ )
# uFLIP also includes some lines from the pseudo random generator from Agner
# Fog (see http://www.agner.org/random/)
#
# © 2008-2009, Luc Bouganim <luc.bouganim@inria.fr>
# © 2009, Lionel Le Folgoc <lionel.le_folgoc@inria.fr>
"""
This file provides a function to prepare uFLIP's working directory.
"""

import os
from shutil import copy
import platform

# get the platform to guess the binary name
bin_name = u"FlashIO"
if platform.system() == "Windows":
    bin_name = "%s.%s" % (bin_name, u"exe")

# a directory name is needed
print(u"Directory name for running the benchmark?")
path = raw_input(u"-->")
if len(path) == 0:
    exit(1)

# ask confirmation
print(u"All files will be created in directory %s - confirm (y/n)" % path)
resp = raw_input(u"-->")
if resp != u"y":
    exit(1)

# let's begin
os.mkdir(path)

# copy some required files
for f in [u"clean.py", u"ExpPlan.txt", u"ExpSel.txt", bin_name]:
    copy(f, path)

# prepare the arborescence
for d in [u"TRACES", u"DETAILS", u"TIMINGS", u"RESULTS", u"FILES"]:
    os.makedirs(os.path.join(path, d))

# required files, again
copy(u"empty.csv", os.path.join(path, u"FILES"))
copy(u"create.py", os.path.join(path, u"RESULTS"))
copy(u"process.py", os.path.join(path, u"RESULTS"))
copy(u"_Vizu_V3.xls", os.path.join(path, u"RESULTS"))

# do some cleaning
import clean
clean.clean(path)

print(u"Setup finished")
print(u"Now, you should run FlashIO to generate the prepare batch:")
print(u"FlashIO GenPrepare Dev <your_device> IOS 64 IOC 10000 IOC2 50000")
print(u"then, run the batch Prepare.{bat,sh} (see readme.pdf)")
print(u"For having help, type FlashIO help")
raw_input(u"Press RETURN.")
exit(0)
