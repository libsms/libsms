#!/bin/env python

from pylab import *
from pysms import *
from numpy import array

windowType = SMS_WIN_IFFT;

sizeWin = 512;
window = zeros(sizeWin);
sms_getWindow(window,windowType);

plot(window);
show();
