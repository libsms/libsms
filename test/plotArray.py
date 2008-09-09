#!/usr/bin/env python

from pylab import *
import sys

filename = sys.argv[1]

print "plotting:", filename

values = [ float(x) for x in open(filename).read().split() ]
plot(values)

show()
