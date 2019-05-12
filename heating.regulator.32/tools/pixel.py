#!/usr/bin/env python2.7

from __future__ import print_function
from PIL import Image

i = Image.open("lock.gif")
ix, iy = i.size
p = i.load()

print ("const unsigned int flame[%s] PROGMEM={" %  (ix * iy))
bytes = "    "
for y in range(0, iy):
    line = "    "
    for x in range(0, ix):
        bytes += "%s, " % (1 if not p[x, y] else 0)
    bytes += "\n" + line

print (bytes[:-7] + "\n};") 
