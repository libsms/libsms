#!/usr/bin python

from pylab import *
from pysms import *

filename = "fs.sms"

file = SMS_File()
file.load(filename)

nFrames = file.header.nFrames
nTracks = file.header.nTracks
freq = zeros([ nTracks, nFrames])
amp = freq.copy()
# phase = freq.copy()

subplot(111,axisbg=[0,0,0])
timebar = arange(nFrames) / float(file.header.iFrameRate)
for i in xrange(nTracks):
    file.getTrack(i, freq[i], amp[i])
    if freq[i].sum() != 0:
        blarg = plot(timebar, freq[i], 'w')

file.close()
show()
