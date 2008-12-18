#!/bin/env python

from pylab import *
from sms import *

windowType = SMS_WIN_IFFT;

sizeWin = 512;
pFWindow = floatArray(sizeWin)

sms_getWindow(sizeWin, pFWindow,windowType);

window = [0]*sizeWin;

for i in range(sizeWin):
    window[i] = getFloat(pFWindow,i);

plot(window);
show();
