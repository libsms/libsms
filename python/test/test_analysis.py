#!/usr/bin python

from pysms import *
from pylab import *

filename = "../../test/audio/flugel.wav"
sizeWin = 301
sizeSpec = 512
winType = SMS_WIN_HAMMING
sample = 1000
# class pysms_error(Exception):
#     def __init__(self, value):
#         self.value = value
#             print "","An Expletive occured!"
#         def __str__(self):
#             print "","An Expletive occured!"
            

soundheader = SMS_SndHeader()
#smsData = SMS_Data()
analParams = SMS_AnalParams()
sms_initAnalParams(analParams))

analFrame = SMS_AnalFrame()

wave = zeros(sizeWin).astype('float32')
Mag = zeros(sizeSpec).astype('float32')
Phase = zeros(sizeSpec).astype('float32')
win = zeros(sizeWin).astype('float32')
sms_getWindow(win, winType)

if(sms_openSF(filename, soundheader)):
    err = "error opening sound file: " + sms_errorString()
    #raise fileError(err)
    raise NameError(err)


nSamples = soundheader.nSamples

if sms_getSound(soundheader, wave, sample):
    err = "error opening sound file: " + sms_errorString()
    raise NameError(err)


err = sms_spectrum(wave, win,Mag, Phase)

sms_arrayMagToDB(Mag)

#next:
sms_detectPeaks(Mag, Phase, analFrame.pSpectralPeaks
