# here is a failing attempt at an stft function... tofix
from numpy import *

#def stft(x, nfft, fs, sizeWindow, noverlap):      
  


noverlap=256
sizeWindow=512
nfft=2048
dbclip=100
srate = Fs
nframes = size(x)
# make sure signal is at least 1 window length
if nframes < sizeWindow:
    x = hstack([x, zeros(sizeWindow - nframes)])

# set overlap/hopsize
if noverlap < 0:
    nhop = - noverlap
    noverlap = sizeWindow - nhop
else:
    nhop = sizeWindow - noverlap

# make zero-padded windowing function
Modd = mod(sizeWindow,2) # 0 if sizeWindow even, 1 if odd
Mo2 = (sizeWindow-Modd)/2
window=hamming(sizeWindow)
zp = zeros(nfft-sizeWindow)
wzp = hstack([ window[Mo2:sizeWindow], zp, window[0:Mo2] ]) 

# count frames, make an STFT buffer and fill with FFTs
frames = 1 + floor((nframes-noverlap)/nhop)
X = zeros((frames, nfft))
xoff = 0

#for i in xrange(frames):
#    xframe = array(x[xoff:xoff+sizeWindow])
#    xoff = xoff + nhop
#    xzp = hstack([xframe[Mo2:sizeWindow], zp, xframe[0:Mo2] ])
#    xw = wzp * xzp
#    X[i] = fft.fft(xw)


#return(X)
