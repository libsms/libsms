#!/usr/bin python

from pylab import *
from pysms import *

#filename = "../../test/audio/fs.wav
filename = "../../test/audio/soopastar.wav"
#filename = "../../test/audio/tone440.wav"
sizeWin = 301
sizeSpec = 512
winType = SMS_WIN_HAMMING
#winType = SMS_WIN_HANNING
sample = 10000 #0
soundheader = SMS_SndHeader()
smsData = SMS_Data()
wave = zeros(sizeWin)
Mag = zeros(sizeSpec)
Phase = zeros(sizeSpec)
win = zeros(sizeWin)
sms_getWindow(win, winType)
sms_scaleWindow(win)
##############################
### Open / Get Sound ###########
sms_openSF(filename, soundheader)
nSamples = soundheader.nSamples
sms_getSound(soundheader, wave, sample);

# testing window centering:
#buff = zeros(sizeSpec*2).astype('float32')
#win2 = zeros(sizeWin+2).astype('float32')
#sms_windowCentered(wave, win2, buff)

################################
### compute spectrum of a frame #####
sms_setMagThresh(.01)
sms_spectrum(wave, win,Mag, Phase)
dbMag = Mag.copy()
sms_arrayMagToDB(dbMag)
#sms_arrayDBToMag(Mag)

#newWave = zeros(sizeWin).astype('float32')

# humm.. inverse not working, why is there still 1/2 zeros? this should be
# undoing the zero padding too.
#err = sms_invSpectrum(newWave, win, Mag, Phase)

###############################
### get partial peaks #############
#nPeaks = 39
nPeaks = 30
print "nPeaks: ", nPeaks
peakParams = SMS_PeakParams()
peakParams.fHighestFreq = 20000
peakParams.fLowestFreq = 100
peakParams.fMinPeakMag = 
peakParams.iSamplingRate = soundheader.iSamplingRate
peakParams.iMaxPeaks = nPeaks
peaks = SMS_SpectralPeaks(nPeaks)

sms_detectPeaks(dbMag, Phase, peaks, peakParams)
nPeaks = peaks.nPeaksFound

fx = zeros(nPeaks)
ax = zeros(nPeaks)

peaks.getFreq(fx)  # why aren't I getting any peaks > 10k hertz?
peaks.getMag(ax)

# plot the peaks on top of the magnitude spectrum
if True:
    hold(False)
    subplot(111, axisbg='black')
    faxis = arange(sizeSpec) * soundheader.iSamplingRate *.5 / sizeSpec
    plot(faxis,dbMag)
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
