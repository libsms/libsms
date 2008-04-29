dt#!/usr/bin/env python
from pylab import *
import aifc, yaml, numpy

soundFileName = 'audio/piano5.aiff'
yamlFileName = 'piano5.yaml'

print 'reading', soundFileName, '...'
sf = aifc.open(soundFileName, 'r')

    
bits = sf.getsampwidth()*8
srate = sf.getframerate()
nframes = sf.getnframes()

if sf.getnchannels() == 2:
    sys.exit("stereo file not yet supported, TODO: mix-to-mono")

# get data and convert to unsigned int from hex string
sfdata = numpy.fromstring(sf.readframes(nframes), 'H')
# byteswap and normalize to 0:+2^bits
sfdata = sfdata.byteswap() - 2**bits / 2
# convert to float and normalize to -1:1
sfdata = array(sfdata, float) / (.5 * 2**bits) - 1


#now plot a spectragram of sfdata
NFFT = 1024       # the length of the windowing segments
# Pxx is the segments x freqs array of instantaneous power, freqs is
# the frequency vector, bins are the centers of the time bins in which
# the power is computed, and im is the matplotlib.image.AxesImage
# instance

Pxx, freqs, bins, im = specgram(sfdata, NFFT=NFFT, Fs=srate, noverlap=900)

# show()


#import sms analysis data from yaml file
print 'loading', yamlFileName, '...'
sms = yaml.load(open(yamlFileName).read())

nRecords = sms['header']['nRecords']

