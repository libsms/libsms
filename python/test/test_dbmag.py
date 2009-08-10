#! /bin/env python

from pylab import *
from pysms import *

filename = "../../test/audio/tone440.wav"
sizeWin = 301
sizeSpec = 512
winType = SMS_WIN_HAMMING
sample = 0
soundheader = SMS_SndHeader()
smsData = SMS_Data()
wave = zeros(sizeWin)
Mag = zeros(sizeSpec)
Phase = zeros(sizeSpec)
win = zeros(sizeWin)
sms_getWindow(win, winType)
sms_scaleWindow(win)
sms_openSF(filename, soundheader)
nSamples = soundheader.nSamples
sms_getSound(soundheader, wave, sample);

sms_spectrum(wave, win,Mag, Phase)

sms_setMagThresh(.2)
dbMag = Mag.copy()
sms_arrayMagToDB(dbMag)
Mag2 = dbMag.copy()
sms_arrayDBToMag(Mag2)

hold(False);
subplot(311);
plot(Mag);
subplot(312);
plot(dbMag);
subplot(313);
plot(Mag2);
show()
g
# now make a dbMag with a threshold
thresh = .3
norm = 100 + 20. * log10(.3)
dbMag2 = zeros(len(Mag))
#dbMag2(Mag> thresh) = norm + 20.*log10(Mag / thresh)

#dbMag2[nonzero(Mag>thresh)] = 20*log10(Mag[nonzero(Mag>thresh)]/thresh) #- 20*log10(thresh)
