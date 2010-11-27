#!/usr/bin python

from pylab import *
from pysms import *

##########################
#####  parameters  ##########
maxFreq = 5000

filename = "fs.wav"
sizeWin = 601
sizeSpec = 512
winType = SMS_WIN_HAMMING
sample = 1000

soundheader = SMS_SndHeader()
smsData = SMS_Data()
wave = zeros(sizeWin)
Mag = zeros(sizeSpec)
Phase = zeros(sizeSpec)
win = zeros(sizeWin)
sms_getWindow(win, winType)
sms_scaleWindow(win)
#sms_setMagThresh(.001)
##############################
### Open / Get Sound ###########
sms_openSF(filename, soundheader)
nSamples = soundheader.nSamples
sms_getSound(soundheader, wave, sample)
sr = soundheader.iSamplingRate
################################
### compute spectrum of a frame #####
sms_spectrum(wave, win,Mag, Phase);
sms_arrayMagToDB(Mag)

###############################
### get partial peaks #############
#nPeaks = 22
#nPeaks = 5
peakParams = SMS_PeakParams()
peakParams.fHighestFreq = maxFreq
peakParams.fLowestFreq = 0.
peakParams.fMinPeakMag = -90.
#peakParams.fMinPeakMag = -10.
peakParams.iSamplingRate = soundheader.iSamplingRate
peakParams.iMaxPeaks = 100
peaks = SMS_SpectralPeaks(peakParams.iMaxPeaks)
sms_detectPeaks(Mag, Phase, peaks, peakParams)
fx = zeros(peaks.nPeaks)
ax = zeros(peaks.nPeaks)
peaks.getFreq(fx)  # why aren't I getting any peaks > 10k hertz?
peaks.getMag(ax)

    
################################################
### compute discrete cepstrum envelope from partial peaks

npoints = peaks.nPeaksFound
Xk = 10**(ax[0:peaks.nPeaksFound -1]/20.)
order = 25
#order = 3
lam = 0.00001
#lam = .00000000001; # has to be something, or matrix M isn't positive definite

#maxBin = sms_power2(int(float(maxFreq) * sizeSpec * 2 / soundheader.iSamplingRate))
maxBin = 512
cepstrum = zeros(order)
specEnv = zeros(maxBin)

sms_dCepstrum(cepstrum, fx[0:npoints-1], Xk, lam, maxFreq)
sms_dCepstrumEnvelope(cepstrum, specEnv) 
env_db = 20*log10(specEnv)


###############################################
# plot the magnitude spectrum, peaks, and envelope
#maxfreq = peakParams.fHighestFreq
N = sizeSpec*2
maxbin = maxFreq * N / sr;
fRes = float(sr) / N
if True:
    hold(False)
    subplot(111, axisbg='black')
    f_axis = arange(0, maxFreq - fRes*2, fRes)
    plot(f_axis, Mag[0:maxbin-1]) 
    hold(True)
    plot(fx, ax, 'r+')
    env_freq = arange(0, maxFreq, float(maxFreq) / maxBin)
    print len(env_freq)
    print len(env_db)
    plot(env_freq, env_db,'g')
    show()













