#!/usr/bin python

from pylab import *
from matplotlib.widgets import Slider
from sys import path
path.append('../')
from pysms import *

filename = "../../../test/audio/soopastar.wav"
##########################
#####  parameters  ##########
class TestCepstrumEnv:
    def __init__(self, sizeWin=601, sizeSpec = 512, winType = SMS_WIN_BH_70,
                 maxFreq = 5000, order = 25, lam = 0.00001):
        self.maxFreq = maxFreq
        self.sizeWin = sizeWin
        self.sizeSpec = sizeSpec
        self.winType = winType
        self.sample = 0
        self.order = order
        self.lam = lam
        self.maxFreq = maxFreq
        self.sh = SMS_SndHeader()
        self.smsData = SMS_Data()
        self.wave = zeros(sizeWin)
        self.Mag = zeros(sizeSpec)
        self.Phase = zeros(sizeSpec)
        self.win = zeros(sizeWin)
        sms_getWindow(self.win, winType)
        sms_scaleWindow(self.win)
        self.peakParams = SMS_PeakParams()
        self.peakParams.fHighestFreq = maxFreq
        self.peakParams.fLowestFreq = 0.
        self.peakParams.fMinPeakMag = 0.
        self.peakParams.iMaxPeaks = 100
        self.peaks = SMS_SpectralPeaks(self.peakParams.iMaxPeaks)
        self.cepstrum = zeros(order)
        # plot init
        #self.cax = subplot(111)
        subplots_adjust(left=0.1, bottom=0.25)
        #self.axamp  = axes([0.25, 0.15, 0.65, 0.03], axisbg=axcolor)
        #self.samp = Slider(axamp, 'Amp', 0.1, 10.0, valinit=a0)
    def loadFile(self, filename):
        self.filename = filename
        sms_openSF(filename, self.sh)
        sr = self.sh.iSamplingRate
        print "maxbin before: ", self.maxFreq * (self.sizeSpec*2) / sr
        self.maxBin = sms_power2(self.maxFreq * (self.sizeSpec*2) / sr)
        self.peakParams.iSamplingRate = sr

        self.maxFreq = self.maxBin * sr / (self.sizeSpec*2)
        self.peakParams.fHighestFreq = self.maxFreq
        print "maxbin after: ", self.maxBin
        print "maxfreq after: ", self.maxFreq
        self.specEnv = zeros(self.maxBin)
        # set up time slider
        nSeconds = self.sh.nSamples / float(sr) 
        axcolor = 'lightgoldenrodyellow'

        self.axtime = axes([0.2, 0.15, 0.65, 0.03], axisbg=axcolor)
        self.axorder = axes([0.2, 0.10, 0.65, 0.03], axisbg=axcolor)
        self.axlambda = axes([0.2, 0.05, 0.65, 0.03], axisbg=axcolor)

        self.stime = Slider(self.axtime, 'Time (seconds)', 0., nSeconds, valinit=0.05)
        self.stime.on_changed(self.updateFrame)

        self.sorder = Slider(self.axorder, 'Order (p)', 0., 100, valinit=20,  valfmt='%d')
        self.sorder.on_changed(self.updateOrder)

        self.slambda = Slider(self.axlambda, 'lambda (power)', -10., -2, valinit=-5)
        self.slambda.on_changed(self.updateLambda)


    def updateLambda(self, val):
        self.lam = 10**(val)
        print "lambda:", self.lam
        self.compute(self.sample)
        self.plot()
        
    def updateFrame(self, val):
        # set sample here
        self.sample = int(val * self.sh.iSamplingRate)
        print "sample: ", self.sample
        self.compute(self.sample)
        self.plot()
    def updateOrder(self, val):
        # set sample here
        self.order = int(val)
        print "order: ", self.order
        self.cepstrum = zeros(self.order)
        self.compute(self.sample)
        self.plot()
    def compute(self, sample):
        if sample < 0: sample = 0
        if sample + self.sizeWin > self.sh.nSamples: 
            self.sample = self.sh.nSamples - sizeWin -1
        sms_getSound(self.sh, self.wave, sample)
        sms_spectrum(self.wave, self.win, self.Mag, self.Phase);
        #self.Mag = 20*log10(self.Mag)
        sms_arrayMagToDB(self.Mag)
        # get partial peaks #############
        sms_detectPeaks(self.Mag, self.Phase, self.peaks, self.peakParams)
        self.fx = zeros(self.peaks.nPeaks)
        self.ax = zeros(self.peaks.nPeaks)
        self.peaks.getFreq(self.fx)  # why aren't I getting any peaks > 10k hertz?
        self.peaks.getMag(self.ax)
        # compute discrete cepstrum envelope from partial peaks
        npoints = self.peaks.nPeaksFound
        Xk = 10**(self.ax[0:self.peaks.nPeaksFound -1]/20.)
        sms_dCepstrum(self.cepstrum, self.fx[0:npoints-1], Xk, self.lam, self.maxFreq)
        sms_dCepstrumEnvelope(self.cepstrum, self.specEnv) 
        self.specEnv = 20*log10(self.specEnv)
        #sms_arrayMagToDB(self.specEnv)
        #print "env (size: %d): " % (maxBin)
        #print env_db
        #print "cepstrum:"
        #print cepstrum
    def plot(self):
        """ plot the magnitude spectrum, peaks, and envelope
        """
        hold(False)
        N = self.sizeSpec*2
        sr = self.sh.iSamplingRate
        fRes = float(sr) / N 
        subplot(211, axisbg='black')
        plot(self.wave)
        subplot(212, axisbg='black')
        f_axis = arange(0, self.maxFreq - fRes*2, fRes)
        plot(f_axis, self.Mag[0:self.maxBin-2]) 
        ylabel('magnitude (decibels)')
        xlabel('frequency (hertz)')
        #plot(self.Mag[0:self.maxBin-1]) 
        #return
        hold(True)
        plot(self.fx, self.ax, 'r+')
        env_freq = arange(0, self.maxFreq, float(self.maxFreq) / self.maxBin)
        #print "len env_freq: ", len(env_freq)
        #print "len specEnf", len(self.specEnv)
        plot(env_freq, self.specEnv,'g')
        #show()

if __name__ == "__main__":
    c = TestCepstrumEnv(maxFreq=3000)
    c.loadFile(filename)
    c.compute(1000)
    c.plot()
    show()
    #print c.sizeSpec







