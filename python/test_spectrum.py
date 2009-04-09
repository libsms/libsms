#!/usr/bin python

from pylab import *
from pysms import *

filename = "../../test/audio/fs.wav"
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

##############################
### Open / Get Sound ###########
sms_openSF(filename, soundheader)
nSamples = soundheader.nSamples
sms_getSound(soundheader, wave, sample)

# testing window centering:
#buff = zeros(sizeSpec*2).astype('float32')
#win2 = zeros(sizeWin+2).astype('float32')
#sms_windowCentered(wave, win2, buff)

################################
### compute spectrum of a frame #####
err = sms_spectrum(wave, win,Mag, Phase)

sms_arrayMagToDB(Mag)
#sms_arrayDBToMag(Mag)

#newWave = zeros(sizeWin).astype('float32')

# humm.. inverse not working, why is there still 1/2 zeros? this should be
# undoing the zero padding too.
#err = sms_invSpectrum(newWave, win, Mag, Phase)

###############################
### get partial peaks #############
#nPeaks = 39
nPeaks = 5
print "nPeaks: ", nPeaks
peakParams = SMS_PeakParams()
peakParams.fHighestFreq = 20000
peakParams.fLowestFreq = 100
peakParams.fMinPeakMag = 0
peakParams.iSamplingRate = soundheader.iSamplingRate
peakParams.iMaxPeaks = nPeaks
peaks = SMS_SpectralPeaks(nPeaks)

n = sms_detectPeaks(Mag, Phase, peaks, peakParams)

fx = zeros(nPeaks)
ax = zeros(nPeaks)

peaks.getFreq(fx)  # why aren't I getting any peaks > 10k hertz?
peaks.getMag(ax)

# plot the peaks on top of the magnitude spectrum
if False:
    hold(False)
    subplot(111, axisbg='black')
    faxis = arange(sizeSpec) * soundheader.iSamplingRate *.5 / sizeSpec
    plot(faxis,Mag)
    hold(True)
    plot(fx,ax,'r.')


################################################
### compute discrete cepstrum envelope from partial peaks

sms_arrayDBToMag(ax) # this is redunant, but will work for now
ax = log(ax) # .. but will probably also cause numerical errors when amp is near 0
#order = 20
order = 3
print "order: ", order
lam = 0.00001
sr = 8000
cepstrum = zeros(order)
sms_dCepstrum(cepstrum, fx, ax, lam, sr)
print "cepstrum coefficients: ", cepstrum
