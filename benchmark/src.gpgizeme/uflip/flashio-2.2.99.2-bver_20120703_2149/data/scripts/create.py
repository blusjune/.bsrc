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
This file provides a function to create a global results file.
"""

import glob

# Concatenate all results
outfile = open(u"ALL_RESULTS.csv", u"w")
for f in glob.glob(u"RES*"):
    infile = open(f,'r')
    data = infile.read()
    infile.close()
    outfile.write(data)
outfile.close()

# process them
import process
