#!/usr/bin python

from pysms import *
from pylab import *

filename = "../../test/audio/flugel.wav"
sizeWin = 301
sizeSpec = 512
#winType = SMS_WIN_HAMMING
winType = SMS_WIN_HANNING
sample = 1000
soundheader = SMS_SndHeader()
smsData = SMS_Data()
wave = zeros(sizeWin).astype('float32')
Mag = zeros(sizeSpec).astype('float32')
Phase = zeros(sizeSpec).astype('float32')
win = zeros(sizeWin).astype('float32')
sms_getWindow(win, winType)

sms_openSF(filename, soundheader)

nSamples = soundheader.nSamples

sms_getSound(soundheader, wave, sample)

# testing window centering:
#buff = zeros(sizeSpec*2).astype('float32')
#win2 = zeros(sizeWin+2).astype('float32')
#sms_windowCentered(wave, win2, buff)

err = sms_spectrum(wave, win,Mag, Phase)

#sms_arrayMagToDB(Mag)

#sms_arrayDBToMag(Mag)

newWave = zeros(sizeWin).astype('float32')

# humm.. not working, why is there still 1/2 zeros? this should be
# undoing the zero padding too.
err = sms_invSpectrum(newWave, win, Mag, Phase)

# peak detection:
peakParams = SMS_PeakParams()
peakParams.fHighestFreq = 10000
peakParams.fLowestFreq = 100
peakParams.fMinPeakMag = 0
peakParams.iSamplingRate = soundheader.iSamplingRate
peakParams.iMaxPeaks = 10

peaks = zeros([peakParams.iMaxPeaks, 3])
nPeaks = sms_detectPeaks(Mag, Phase, peaks, peakParams)

