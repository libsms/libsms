#!/usr/bin/env python


# now using libsndfile-python (http://arcsin.org/softwares/libsndfile-python.html)
import sndfile
#import aifc,  wave, sndhdr
from pylab import *
from numpy import *
# using the libyaml loader is much faster, if available
from yaml import load, dump
try:
    from yaml import CLoader as Loader
    from yaml import CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

soundFilename ='/home/r/samples/instrumental/horn/Flugel/Flug-d5.aiff'
#soundFilename = 'audio/piano.aiff'
yamlFilename = '../tools/flugel.yaml'
#yamlFilename = 'piano.yaml'

sf = sndfile.open(soundFilename, 'r')
sfinfo = sf.get_info()
if sfinfo.channels == 2:
    print 'stero files not yet supported'

sndfile.set_numarray_mode(1)

srate = sfinfo.samplerate
nframes = sfinfo.frames

sfdata = sf.readf_float(nframes)


# plot a spectragram of sound
plotTitle = 'sample: '+ soundFilename

Pxx, freq, bins, im = specgram(sfdata, NFFT=2048, Fs= srate, noverlap=512)
title(plotTitle)
xlabel('time (seconds)')
ylabel('frequency (hertz)')
axes().set_ylim(0,5000)
show()



#import sms analysis data from yaml file
#print 'loading', yamlFileName, '...'
smsFile = load(open(yamlFilename).read(), Loader=Loader)

nRecords = smsFile['smsHeader']['nRecords']
nTraj = smsFile['smsHeader']['nTrajectories']

smsData = smsFile['smsData'] 

# make a track list: tracks[[ [timetags], [freqs] ]]
tracks = [0] *nTraj
for i in range(nTraj): # loop for each harmonic
    time = 0; # use int index to check if it has been set (which would be positive)
    freq = 0;
    for j in range(nRecords): # loop for all frames
        if smsData[j]['harmonics']: # make sure frame has harmonic data #TODO: check if necessary loop
            for num,traj in smsData[j]['harmonics'].iteritems(): #  build time and freq lists
                if i == num:
                    if time == 0:
                        time = [smsData[j]['timetag']]
                        freq = [smsData[j]['harmonics'][num][0]]
                    else:
                        time.append(smsData[j]['timetag'])
                        freq.append(smsData[j]['harmonics'][num][0])
        tracks[i] = [time,freq]
   
for i in range(nTraj): # if traj has data, plot it
    if tracks[i][0] != 0:
        plot(tracks[i][0],tracks[i][1])


# make an array of stocWave frames for one continuous waveform
if smsFile['smsHeader']['iStochasticType'] == 'waveform' :
    resWave = []
    for i in range(nRecords):
        resWave += smsData[i]['stocWave']

    Pxx, freq, bins, im = specgram(resWave, NFFT=2048, Fs= srate, noverlap=512)

if smsFile['smsHeader']['iStochasticType'] == 'approx' :
    print 'approx YEAAHH'
