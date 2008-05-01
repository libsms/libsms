#!/usr/bin/env python

import aifc
from pylab import *
from numpy import *
# using the libyaml loader is much faster, if available
from yaml import load, dump
try:
    from yaml import CLoader as Loader
    from yaml import CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

soundFilename = 'audio/piano.aiff'
yamlFilename = 'piano5.yaml'


sf = aifc.open(soundFilename, 'r')

bits = sf.getsampwidth()*8
srate = sf.getframerate()
nframes = sf.getnframes()

# get data and convert to unsigned int from hex string
sfdata = fromstring(sf.readframes(nframes), 'H')
# byteswap and normalize to 0:+2^bits
sfdata = sfdata.byteswap() - 2**bits / 2
# convert to float and normalize to -1:1
sfdata = array(sfdata, float) / (.5 * 2**bits) - 1

# plot a spectragram of sound
ymax = 5000
plotTitle = 'sample:', soundFilename

specgram(sfdata, NFFT=1024, Fs= srate, noverlap=900)
title(plotTitle)
xlabel('time (seconds)')
ylabel('frequency (hertz)')
axes().set_ylim(0,ymax)
show()



#import sms analysis data from yaml file
#print 'loading', yamlFileName, '...'
smsFile = load(open(yamlFilename).read(), Loader=Loader)

nRecords = smsFile['header']['nRecords']
nTraj = smsFile['header']['nTrajectories']

smsData = smsFile['Data'] #todo: change to 'data' for future files

# make a track and plot [num,[timetags],[freqs]]
for i in range(nTraj): # loop for each harmonic
    time = -1; # use -1 index to check if it has been set (which would be positive)
    freq = 0;
    for j in range(nRecords): # loop for all frames
        if smsData[j]['harmonics']: # make sure frame has harmonic data
            for num,traj in smsData[j]['harmonics'].iteritems(): #  build time and freq lists
                if i == num:
                    if time == -1:
                        print 'setting first frame'
                        time = [smsData[j]['timetag']]
                        freq = [smsData[j]['harmonics'][num][0]]
                    else:
                        time.append(smsData[j]['timetag'])
                        freq.append(smsData[j]['harmonics'][num][0])
    plot(time,freq)

        


