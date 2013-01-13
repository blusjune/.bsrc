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
# © 212, Brian M. JUNG <brian.m.jung@gmail.com>
"""
This file provides a method to parse all results and generate graphs.
"""

import csv # This will help us reading csv formated files
import copy
import math

averaged = [2, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27]

f = open('ALL_RESULTS.csv', 'rb')
# The reader method will put each line
# of the csv file into a list of columns.
reader = csv.reader(f, delimiter=';')
g = open('ExcelBase.csv', 'wb')
writer = csv.writer(g, delimiter=';')

NbLine = 1
for line in reader:
    writer.writerow(line)
    NbLine = NbLine + 1

while NbLine < 1700:
    NbLine = NbLine + 1
    writer.writerow("")
   
f.close()
#print 'end of copy'
f = open('ALL_RESULTS.csv', 'rb') 
reader = csv.reader(f, delimiter=';')

NbLine = 1    
nbMeasure = 1
line2 = reader.next()
#print line2
writer.writerow(line2)  
line2 = reader.next()
#print line2
listVal = []
if (line2[0] != 'Comment'):
    listVal.append(float(line2[17]))
    #print 'liste = ', listVal
NbL = 1
for line in reader:
    #print line
    if (len(line) < 10):
        line[0] = 'Comment'
        line.append('-')
        line.append('-')
        line.append('-')
        line.append('-')
        line[3] = ' RunID' 
    #print line
    NbL = NbL + 1
    if (line2[0] == 'Comment'):
        while NbLine < 100:
            NbLine = NbLine + 1
            writer.writerow("")
        line2[3] = 'StdD_Run'
        writer.writerow(line2)
        NbLine = 1
        line2 = copy.copy(line)
        if (line[0] != 'Comment'):
            listVal.append(float(line[17]))
            #print 'liste = ', listVal
    else:
        #print 'same run'
        sameExp = 1
        #print 'par', line[4], (line[4] == ' 0')
        #print 'run', line[3], (line[3] != '   0') , (line[3] != ' RunID')
        if (((line[4] == ' 0') and (line[3] != '   0') and (line[3] != ' RunID'))
            or ((line[4] != ' 0') and (line[4] == line2[4]))):
            nbMeasure = nbMeasure + 1
            #print 'adding', line[17]
            listVal.append(float(line[17]))
            #print 'liste = ', listVal
            for j in averaged:
                line2[j] = float(line2[j]) + float(line[j])
        else:
            #print 'end of run'
            for j in averaged:
                line2[j] = int(float(line2[j]) / float(nbMeasure))
            std = 0
            nb = 0
            #print listVal
            for j in listVal:
                std = std + (j - float(line2[17])) * (j - float(line2[17]))
                nb = nb + 1
                #print nb, j, std
            std = math.sqrt(std/nb)
            line2[3] = std
            NbLine = NbLine + 1
            writer.writerow(line2)   
            line2 = copy.copy(line)
            listVal = []
            if (line[0] != 'Comment'):
                print line[0];
                print line[1];
                print line[17];
                listVal.append(float(line[17]))
                #print 'liste = ', listVal
            nbMeasure = 1

    
#    print line[1],   
#    print line[2],   
#    print line[3],   
#    print line[4],   
#    print line[5],   
#    print "\n"
f.close()
