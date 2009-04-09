#!/usr/bin python

from pylab import *
from pysms import *

#filename = "../tools/flugel.sms"
filename = "/home/r/samples/sms/guitarchord-strange.sms"

file = SMS_File()
file.load(filename)

nFrames = file.header.nFrames
print "nFrames: ", nFrames
nTracks = file.header.nTracks
freq = zeros([ nFrames, nTracks])
amp = freq.copy()

subplot(111,axisbg=[0,0,0])
timebar = zeros(nTracks)
incr = 1.0 / file.header.iFrameRate
for i in xrange(nFrames):
    file.getFrameDet(i, freq[i], amp[i])
#     if freq[i][0] != 0:
#         blarg = plot(timebar, freq[i], 'w_')
    blarg = plot(timebar, freq[i], 'w_')
    timebar += incr

file.close()
show()
