#!/bin/env python

from pylab import *
from pysms import *
from numpy import array

windowType = SMS_WIN_IFFT;

sizeWin = 512;
window = zeros(sizeWin,dtype=float);
sms_getWindow(sizeWin, window,windowType);

window = [0]*sizeWin;

for i in range(sizeWin):
    window[i] = getFloat(pFWindow,i);

plot(window);
show();
