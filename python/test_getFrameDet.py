#!/usr/bin python

from pylab import *
from pysms import *

filename = "../tools/flugel.sms"

file = SMS_File()
sms_loadFile(filename, file)

nFrames = file.header.nFrames
nTracks = file.header.nTracks
freq = zeros([ nFrames, nTracks])
amp = freq.copy()
phase = freq.copy()

subplot(111,axisbg=[0,0,0])
timebar = zeros(nTracks)
incr = 1.0 / file.header.iFrameRate
for i in xrange(nFrames):
    sms_getFrameDet(i, file, freq[i], amp[i], phase[i])
    if freq[i][0] != 0:
        blarg = plot(timebar, freq[i], 'w_')
    timebar += incr

sms_closeFile(file)
show()
