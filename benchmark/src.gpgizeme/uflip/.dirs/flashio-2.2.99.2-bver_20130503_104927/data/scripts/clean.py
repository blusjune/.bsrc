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
This file provides a method to clean a uFLIP working directory.
"""

import os
from shutil import copy
import glob

def clean(directory=None):
    """
    Cleans given directory.
    """
    if directory is not None:
        print(u"Clean called on '%s'." % directory)
        os.chdir(directory)

    # ask confirmation
    print(u"Delete all results? (y/n)")
    resp = raw_input(u"-->")
    if resp != u"y":
        exit(1)

    # let's go
    for pattern in [u"_SEM*", os.path.join(u"RESULTS", u"RES_*"),
        os.path.join(u"TIMINGS", u"TIM_*"), os.path.join(u"TIMINGS", u"AVG_*"),
        os.path.join(u"TIMINGS", u"SOR_*"), os.path.join(u"DETAILS", u"DET_*"),
        os.path.join(u"TRACES", u"TRA_*"), os.path.join(u"TRACES", u"ERR_*"),
        os.path.join(u"TRACES", u"TMP_*"), os.path.join(u"TRACES", u"LOG.txt")]:
        for old_file in glob.glob(pattern):
            if os.path.exists(old_file):
                os.remove(old_file)

    src = os.path.join(u"FILES", u"empty.csv")

    file_list = {0:5, 1:4, 2:4, 3:4, 4:2, 5:2, 6:4, 7:6, 8:4, 9:4}

    for k, v in file_list.iteritems():
        for i in xrange(v):
            copy(src, os.path.join(u"RESULTS", u"RES_%d_%d.csv" % (k, i+1)))

    print(u"Clean finished")
    raw_input(u"Press RETURN.")

if __name__ == '__main__':
    clean()
    exit(0)
